/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "work_scheduler_service.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <climits>          // for PATH_MAX

#include <dirent.h>
#include <fcntl.h>
#include <file_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>

#include "parameters.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_proxy.h"
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#endif
#ifdef DEVICE_STANDBY_ENABLE
#include "standby_service_client.h"
#include "allow_type.h"
#endif
#include "conditions/battery_level_listener.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "conditions/condition_checker.h"
#include "conditions/network_listener.h"
#include "conditions/screen_listener.h"
#include "conditions/storage_listener.h"
#include "conditions/timer_listener.h"
#include "conditions/group_listener.h"
#include "config_policy_utils.h"           // for GetOneCfgFile
#include "directory_ex.h"
#include "event_publisher.h"
#include "policy/app_data_clear_listener.h"
#include "policy/memory_policy.h"
#include "policy/thermal_policy.h"
#include "policy/cpu_policy.h"
#ifdef POWERMGR_POWER_MANAGER_ENABLE
#include "policy/power_mode_policy.h"
#endif
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
#include "scheduler_bg_task_subscriber.h"
#include "background_task_mgr_helper.h"
#include "resource_type.h"
#endif
#include "work_datashare_helper.h"
#include "work_scheduler_connection.h"
#include "work_bundle_group_change_callback.h"
#include "work_sched_errors.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "hitrace_meter.h"
#include "hisysevent.h"
#include "res_type.h"
#include "res_sched_client.h"
#include "work_sched_data_manager.h"
#include "work_sched_config.h"
#include "work_sched_constants.h"
#include "work_sched_hisysevent_report.h"

#ifdef HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#define XCOLLIE_TIMEOUT_SECONDS 10
#endif

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";
const std::string PRINSTALLED_WORKS_KEY = "work_scheduler_preinstalled_works";
const std::string EXEMPTION_BUNDLES_KEY = "work_scheduler_eng_exemption_bundles";
const std::string MIN_REPEAT_TIME_KEY = "work_scheduler_min_repeat_time";
auto instance = DelayedSingleton<WorkSchedulerService>::GetInstance();
auto wss = instance.get();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(wss);
const int32_t UID_TRANSFORM_DIVISOR = 200000;
const int32_t INIT_DELAY = 2 * 1000;
const int32_t CHECK_CONDITION_DELAY = 5 * 1000;
const int32_t MAX_BUFFER = 2048;
const int32_t DUMP_OPTION = 0;
const int32_t DUMP_PARAM_INDEX = 1;
const int32_t DUMP_VALUE_INDEX = 2;
const int32_t TIME_OUT = 4;
const uint32_t SYS_APP_MIN_REPEAT_TIME = 5 * 60 * 1000;
const char* PERSISTED_FILE_PATH = "/data/service/el1/public/WorkScheduler/persisted_work";
const char* PERSISTED_PATH = "/data/service/el1/public/WorkScheduler";
const char* PREINSTALLED_FILE_PATH = "etc/backgroundtask/config.json";
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
static int g_hasGroupObserver = -1;
#endif
const static std::string STRATEGY_NAME = "WORK_SCHEDULER";
const std::set<std::string> WORK_SCHED_NATIVE_OPERATE_CALLER = {
    "resource_schedule_service",
    "hidumper_service",
};

const std::set<std::string> WORK_SCHED_SA_CALLER = {
    "push_manager_service",
};
}

#ifdef WORK_SCHEDULER_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

WorkSchedulerService::WorkSchedulerService() : SystemAbility(WORK_SCHEDULE_SERVICE_ID, true) {}
WorkSchedulerService::~WorkSchedulerService() {}

void WorkSchedulerService::OnStart()
{
    if (ready_) {
        WS_HILOGI("OnStart is ready, nothing to do.");
        return;
    }

    // Init handler.
    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_NAME, AppExecFwk::ThreadMode::FFRT);
    }
    if (eventRunner_ == nullptr) {
        WS_HILOGE("Init failed due to create EventRunner");
        return;
    }
    handler_ = std::make_shared<WorkEventHandler>(eventRunner_, instance);
    if (!handler_) {
        WS_HILOGE("Init failed due to create handler_");
        return;
    }

    // Try to init.
    Init(eventRunner_);
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    AddSystemAbilityListener(DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID);
#endif
#ifdef DEVICE_STANDBY_ENABLE
    AddSystemAbilityListener(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
#endif
    WS_HILOGD("On start success.");
}

WEAK_FUNC bool WorkSchedulerService::IsBaseAbilityReady()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(TIME_SERVICE_ID) == nullptr) {
        return false;
    }
    return true;
}

void WorkSchedulerService::InitPersistedWork()
{
    WS_HILOGD("init persisted work");
    std::lock_guard<ffrt::mutex> lock(mutex_);
    list<shared_ptr<WorkInfo>> persistedWorks = ReadPersistedWorks();
    for (auto it : persistedWorks) {
        WS_HILOGI("get persisted work, id: %{public}d, isSa:%{public}d", it->GetWorkId(), it->IsSA());
        AddWorkInner(*it);
    }
    RefreshPersistedWorks();
}

void WorkSchedulerService::InitPreinstalledWork()
{
    WS_HILOGD("init preinstalled work");
    list<shared_ptr<WorkInfo>> preinstalledWorks = ReadPreinstalledWorks();
    std::lock_guard<ffrt::mutex> lock(mutex_);
    for (auto work : preinstalledWorks) {
        WS_HILOGI("preinstalled workinfo id %{public}s, isSa:%{public}d", work->GetBriefInfo().c_str(), work->IsSA());
        time_t baseTime;
        (void)time(&baseTime);
        work->RequestBaseTime(baseTime);
        AddWorkInner(*work);
        if (work->IsPersisted()) {
            string workId = "u" + to_string(work->GetUid()) + "_" + to_string(work->GetWorkId());
            persistedMap_.emplace(workId, work);
        }
    }
    if (minCheckTime_ && minCheckTime_ < workQueueManager_->GetTimeCycle()) {
        workQueueManager_->SetTimeCycle(minCheckTime_);
    }
}

void WorkSchedulerService::InitWorkInner()
{
    InitPreinstalledWork();
    InitPersistedWork();
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ReadPersistedWorks()
{
    list<shared_ptr<WorkInfo>> workInfos;
    nlohmann::json root;
    if (!GetJsonFromFile(PERSISTED_FILE_PATH, root)) {
        return workInfos;
    }
    if (root.is_null() || root.empty()) {
        WS_HILOGE("ReadPersistedWorks failed, root is empty or not an object");
        return workInfos;
    }
    for (const auto &[key, workJson] : root.items()) {
        shared_ptr<WorkInfo> workInfo = make_shared<WorkInfo>();
        if (!workInfo->ParseFromJson(workJson)) {
            WS_HILOGE("ReadPersistedWorks failed, parseFromJson error");
            continue;
        }
        workInfos.emplace_back(workInfo);
        WS_HILOGI("find one persisted work %{public}s", workInfo->GetBriefInfo().c_str());
        auto iter = std::find_if(persistedMap_.begin(), persistedMap_.end(), [&](const auto &pair) {
            return (pair.second->GetUid() == workInfo->GetUid()) && (pair.second->GetWorkId() == workInfo->GetWorkId());
        });
        if (iter != persistedMap_.end()) {
            WS_HILOGI("find work %{public}s in persisted map, ignore, isSA:%{public}d",
                workInfo->GetBriefInfo().c_str(),
                workInfo->IsSA());
            // update basetime
            continue;
        }
        string workId = "u" + to_string(workInfo->GetUid()) + "_" + to_string(workInfo->GetWorkId());
        persistedMap_.emplace(workId, workInfo);
    }
    return workInfos;
}

void WorkSchedulerService::LoadWorksFromFile(const char *path, list<shared_ptr<WorkInfo>> &workInfos)
{
    if (!path) {
        return;
    }
    nlohmann::json root;
    if (!GetJsonFromFile(path, root) || root.is_null() || root.empty()) {
        WS_HILOGE("file is empty %{private}s", path);
        return;
    }
    if (!root.contains(PRINSTALLED_WORKS_KEY)) {
        WS_HILOGE("no work_scheduler_preinstalled_works key");
        return;
    }
    nlohmann::json preinstalledWorksRoot = root[PRINSTALLED_WORKS_KEY];
    if (preinstalledWorksRoot.empty() || !preinstalledWorksRoot.is_object()) {
        WS_HILOGE("work_scheduler_preinstalled_works content is empty");
        return;
    }
    for (const auto &[key, workJson] : preinstalledWorksRoot.items()) {
        shared_ptr<WorkInfo> workinfo = make_shared<WorkInfo>();
        if (!workinfo->ParseFromJson(workJson)) {
            WS_HILOGE("LoadWorksFromFile failed, parseFromJson error");
            continue;
        }
        if (!workinfo->IsSA()) {
            int32_t uid;
            if (!GetUidByBundleName(workinfo->GetBundleName(), uid)) {
                continue;
            }
            workinfo->RefreshUid(uid);
            preinstalledBundles_.insert(workinfo->GetBundleName());
        }
        workinfo->SetPreinstalled(true);
        workInfos.emplace_back(workinfo);
    }
}

void WorkSchedulerService::LoadExemptionBundlesFromFile(const char *path)
{
    if (!path) {
        return;
    }
    nlohmann::json root;
    if (!GetJsonFromFile(path, root) || root.is_null() || root.empty()) {
        WS_HILOGE("file is empty %{private}s", path);
        return;
    }
    if (!root.contains(EXEMPTION_BUNDLES_KEY)) {
        WS_HILOGE("no work_scheduler_eng_exemption_bundles key");
        return;
    }
    nlohmann::json exemptionBundlesRoot = root[EXEMPTION_BUNDLES_KEY];
    if (exemptionBundlesRoot.empty() || !exemptionBundlesRoot.is_array()) {
        WS_HILOGE("work_scheduler_eng_exemption_bundles content is empty");
        return;
    }

    for (const auto &exemptionBundleName : exemptionBundlesRoot) {
        if (exemptionBundleName.empty() || !exemptionBundleName.is_string()) {
            WS_HILOGE("Item type error");
        } else {
            WS_HILOGI("bundle name:%{public}s", exemptionBundleName.get<std::string>().c_str());
            exemptionBundles_.insert(exemptionBundleName.get<std::string>());
        }
    }
}

void WorkSchedulerService::LoadMinRepeatTimeFromFile(const char *path)
{
    if (!path) {
        return;
    }
    nlohmann::json root;
    if (!GetJsonFromFile(path, root) || root.is_null() || root.empty()) {
        WS_HILOGE("file is empty %{private}s", path);
        return;
    }
    if (!root.contains(MIN_REPEAT_TIME_KEY)) {
        WS_HILOGE("no work_scheduler_min_repeat_time key");
        return;
    }
    nlohmann::json minRepeatTimeRoot = root[MIN_REPEAT_TIME_KEY];
    if (minRepeatTimeRoot.empty() || !minRepeatTimeRoot.is_object()) {
        WS_HILOGE("work_scheduler_min_repeat_time content is empty");
        return;
    }
    if (minRepeatTimeRoot.contains("default") && minRepeatTimeRoot["default"].is_number_unsigned()) {
        minTimeCycle_ = minRepeatTimeRoot["default"].get<uint32_t>();
    }
    if (!minRepeatTimeRoot.contains("special")) {
        WS_HILOGE("no special key");
        return;
    }
    nlohmann::json specialRoot = minRepeatTimeRoot["special"];
    if (specialRoot.empty() || !specialRoot.is_array()) {
        WS_HILOGE("special content is empty");
        return;
    }
    minCheckTime_ = workQueueManager_->GetTimeCycle();
    for (const auto &it : specialRoot) {
        if (!it.contains("bundleName") || !it["bundleName"].is_string() ||
            !it.contains("time") || !it["time"].is_number_unsigned()) {
            WS_HILOGE("special content is error");
            continue;
        }
        uint32_t time = it["time"].get<uint32_t>();
        if (time < SYS_APP_MIN_REPEAT_TIME) {
            WS_HILOGE("bundleName: %{public}s set time: %{public}d not available, must more than %{public}d",
                it["bundleName"].get<std::string>().c_str(), time, SYS_APP_MIN_REPEAT_TIME);
            continue;
        }
        if (minCheckTime_ > time) {
            minCheckTime_ = time;
        }
        specialMap_.emplace(it["bundleName"].get<std::string>(), time);
    }
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ReadPreinstalledWorks()
{
    list<shared_ptr<WorkInfo>> workInfos;
    CfgFiles *files = GetCfgFiles(PREINSTALLED_FILE_PATH);
    if (!files) {
        WS_HILOGE("GetCfgFiles failed");
        WorkSchedUtil::HiSysEventException(EventErrorCode::LOAD_WORK, "get cfg files failed");
        return workInfos;
    }
    // china->base
    for (int i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        LoadWorksFromFile(files->paths[i], workInfos);
        LoadExemptionBundlesFromFile(files->paths[i]);
        LoadMinRepeatTimeFromFile(files->paths[i]);
    }
    FreeCfgFiles(files);
    return workInfos;
}

bool WorkSchedulerService::GetJsonFromFile(const char *filePath, nlohmann::json &root)
{
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(filePath, realPath)) {
        WS_HILOGE("Get real path failed %{private}s", filePath);
        WorkSchedUtil::HiSysEventException(EventErrorCode::LOAD_WORK, "convert real path failed");
        return false;
    }
    WS_HILOGD("Read from %{private}s", realPath.c_str());
    std::string data;
    LoadStringFromFile(realPath.c_str(), data);
    WS_HILOGI("data read success");
    root = nlohmann::json::parse(data, nullptr, false);
    if (root.is_discarded()) {
        WS_HILOGE("parse %{private}s json error", realPath.c_str());
        WorkSchedUtil::HiSysEventException(EventErrorCode::LOAD_WORK, "json parse failed");
        return false;
    }
    WS_HILOGI("json parse success");
    return true;
}

void WorkSchedulerService::OnStop()
{
    WS_HILOGI("stop service.");
    std::lock_guard<ffrt::mutex> observerLock(observerMutex_);
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    DeviceUsageStats::BundleActiveClient::GetInstance().UnRegisterAppGroupCallBack(groupObserver_);
    groupObserver_ = nullptr;
    g_hasGroupObserver = -1;
#endif
#ifdef DEVICE_STANDBY_ENABLE
    DevStandbyMgr::StandbyServiceClient::GetInstance().UnsubscribeStandbyCallback(standbyStateObserver_);
    standbyStateObserver_ = nullptr;
#endif
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*subscriber_);
    if (ret != ERR_OK) {
        WS_HILOGE("unscribe bgtask failed.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_STOP, "unsubscribe background task failed");
    }
#endif
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

bool WorkSchedulerService::Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    if (!IsBaseAbilityReady()) {
        WS_HILOGE("request system service is not ready yet!");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "request system service is not ready");
        GetHandler()->SendEvent(InnerEvent::Get(WorkEventHandler::SERVICE_INIT_MSG, 0), INIT_DELAY);
        return false;
    }
    WorkQueueManagerInit(runner);
    if (!WorkPolicyManagerInit(runner)) {
        WS_HILOGE("init failed due to work policy manager init.");
        return false;
    }
    InitWorkInner();
    if (!Publish(wss)) {
        WS_HILOGE("OnStart register to system ability manager failed!");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "register to system ability manager failed");
        return false;
    }
    checkBundle_ = true;
    ready_ = true;
    WS_HILOGI("start init background task subscriber!");
    if (!InitBgTaskSubscriber()) {
        WS_HILOGE("subscribe background task failed!");
        return false;
    }
    WS_HILOGI("init success.");
    return true;
}

bool WorkSchedulerService::InitBgTaskSubscriber()
{
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    subscriber_ = make_shared<SchedulerBgTaskSubscriber>();
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*subscriber_);
    if (ret != ERR_OK) {
        WS_HILOGE("SubscribeBackgroundTask failed.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "subscribe background task failed");
        return false;
    }
    this->QueryResAppliedUid();
    WS_HILOGD("subscribe background TASK success!");
#endif
    return true;
}

ErrCode WorkSchedulerService::QueryResAppliedUid()
{
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    std::vector<std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo>> appList;
    std::vector<std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo>> procList;
    ErrCode result = BackgroundTaskMgr::BackgroundTaskMgrHelper::GetEfficiencyResourcesInfos(appList, procList);
    if (result != ERR_OK) {
        WS_HILOGE("failed to GetEfficiencyResourcesInfos, errcode: %{public}d", result);
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "get efficiency resources info failed");
        return result;
    }
    std::lock_guard<ffrt::mutex> lock(whitelistMutex_);
    for (const auto& info : appList) {
        if ((info->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) != 0) {
            whitelist_.emplace(info->GetUid());
        }
    }
    for (const auto& info : procList) {
        if ((info->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) != 0) {
            whitelist_.emplace(info->GetUid());
        }
    }
    WS_HILOGI("get efficiency resources infos succeed.");
#endif
    return ERR_OK;
}

void WorkSchedulerService::WorkQueueManagerInit(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    WS_HILOGD("come in");
    if (workQueueManager_ == nullptr) {
        workQueueManager_ = make_shared<WorkQueueManager>(instance);
    }

    auto networkListener = make_shared<NetworkListener>(workQueueManager_);
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    auto chargerListener = make_shared<ChargerListener>(workQueueManager_);
    auto batteryStatusListener = make_shared<BatteryStatusListener>(workQueueManager_);
    auto batteryLevelListener = make_shared<BatteryLevelListener>(workQueueManager_, shared_from_this());
    batteryLevelListener->Start();
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
    auto storageListener = make_shared<StorageListener>(workQueueManager_);
    auto timerListener = make_shared<TimerListener>(workQueueManager_, runner);
    auto groupListener = make_shared<GroupListener>(workQueueManager_, runner);
    auto screenListener = make_shared<ScreenListener>(workQueueManager_, shared_from_this());

    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, networkListener);
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    workQueueManager_->AddListener(WorkCondition::Type::CHARGER, chargerListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_STATUS, batteryStatusListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_LEVEL, batteryLevelListener);
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
    workQueueManager_->AddListener(WorkCondition::Type::STORAGE, storageListener);
    workQueueManager_->AddListener(WorkCondition::Type::TIMER, timerListener);
    workQueueManager_->AddListener(WorkCondition::Type::GROUP, groupListener);
    workQueueManager_->AddListener(WorkCondition::Type::DEEP_IDLE, screenListener);

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    GroupObserverInit();
#endif
    RegisterStandbyStateObserver();
}

bool WorkSchedulerService::WorkPolicyManagerInit(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    WS_HILOGD("come in");
    if (workPolicyManager_ == nullptr) {
        workPolicyManager_ = make_shared<WorkPolicyManager>(instance);
    }
    if (!workPolicyManager_->Init(runner)) {
        WS_HILOGE("work policy manager init failed!");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "work policy manager init failed");
        return false;
    }

#ifdef POWERMGR_THERMAL_MANAGER_ENABLE
    auto thermalFilter = make_shared<ThermalPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(thermalFilter);
#endif // POWERMGR_THERMAL_MANAGER_ENABLE
    auto memoryFilter = make_shared<MemoryPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(memoryFilter);

    auto cpuFilter = make_shared<CpuPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(cpuFilter);

#ifdef POWERMGR_POWER_MANAGER_ENABLE
    auto powerModeFilter = make_shared<PowerModePolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(powerModeFilter);
#endif

    auto appDataClearListener = make_shared<AppDataClearListener>(workPolicyManager_);
    workPolicyManager_->AddAppDataClearListener(appDataClearListener);

    WS_HILOGI("work policy manager init success.");
    return true;
}

WEAK_FUNC bool WorkSchedulerService::GetUidByBundleName(const string &bundleName, int32_t &uid)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::WORK_CHECK, "fail to get system ability manager");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager proxy.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::WORK_CHECK, "fail to get bundle manager proxy");
        return false;
    }
    sptr<IBundleMgr> bundleMgr =  iface_cast<IBundleMgr>(remoteObject);
    BundleInfo bundleInfo;
    int32_t currentAccountId = WorkSchedUtils::GetCurrentAccountId();
    if (bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, currentAccountId)) {
        WS_HILOGD("currentAccountId : %{public}d, bundleName : %{public}s, uid = %{public}d",
            currentAccountId, bundleName.c_str(), bundleInfo.uid);
        uid = bundleInfo.uid;
        return true;
    }
    WS_HILOGE("Get bundle info %{public}s failed.", bundleName.c_str());
    WorkSchedUtil::HiSysEventException(EventErrorCode::WORK_CHECK, "fail to get bundle info");
    return false;
}

bool WorkSchedulerService::GetAppIndexAndBundleNameByUid(int32_t uid, int32_t &appIndex, std::string &bundleName)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager proxy.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr =  iface_cast<IBundleMgr>(remoteObject);
    ErrCode ret = bundleMgr->GetNameAndIndexForUid(uid, bundleName, appIndex);
    if (ret == ERR_OK) {
        WS_HILOGD("appIndex = %{public}d", appIndex);
        return true;
    }
    WS_HILOGE("fail to get app index.");
    return false;
}

bool WorkSchedulerService::CheckExtensionInfos(WorkInfo &workInfo, int32_t uid)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager proxy.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr =  iface_cast<IBundleMgr>(remoteObject);
    BundleInfo bundleInfo;
    if (bundleMgr->GetBundleInfo(workInfo.GetBundleName(),
        BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO,
        bundleInfo, uid / UID_TRANSFORM_DIVISOR)) {
        auto findIter = std::find_if(bundleInfo.extensionInfos.begin(), bundleInfo.extensionInfos.end(),
            [&](const auto &info) {
                WS_HILOGD("%{public}s %{public}s %{public}d", info.bundleName.c_str(), info.name.c_str(), info.type);
                return info.bundleName == workInfo.GetBundleName() &&
                    info.name == workInfo.GetAbilityName() &&
                    info.type == ExtensionAbilityType::WORK_SCHEDULER;
            });
        if (findIter == bundleInfo.extensionInfos.end()) {
            workInfo.RefreshExtension(false);
            WS_HILOGE("extension info is error");
            return false;
        }
    }
    return true;
}

bool WorkSchedulerService::CheckWorkInfo(WorkInfo &workInfo, int32_t &uid)
{
    int32_t appIndex;
    string bundleName;
    if (!GetAppIndexAndBundleNameByUid(uid, appIndex, bundleName)) {
        WS_HILOGE("uid %{public}d is invalid", uid);
        return false;
    }
    workInfo.RefreshAppIndex(appIndex);
    if (workInfo.GetBundleName() != bundleName) {
        WS_HILOGE("bundleName %{public}s is invalid", workInfo.GetBundleName().c_str());
        return false;
    }
    if (!CheckExtensionInfos(workInfo, uid)) {
        WS_HILOGE("workInfo is invalid");
    }
    return true;
}

bool WorkSchedulerService::CheckCondition(WorkInfo& workInfo)
{
    if (workInfo.GetConditionMap()->size() < 1) {
        return false;
    }
    if (workInfo.GetConditionMap()->count(WorkCondition::Type::TIMER) > 0) {
        uint32_t time = workInfo.GetConditionMap()->at(WorkCondition::Type::TIMER)->uintVal;
        string bundleName = workInfo.GetBundleName();
        std::lock_guard<ffrt::mutex> lock(specialMutex_);
        if (specialMap_.count(bundleName) > 0) {
            if (time < specialMap_.at(bundleName)) {
                WS_HILOGE("fail, set time:%{public}u must more than %{public}u", time, specialMap_.at(bundleName));
                return false;
            }
            return true;
        }
        if (time < minTimeCycle_) {
            WS_HILOGE("fail, set time:%{public}u must more than %{public}u", time, minTimeCycle_);
            return false;
        }
    }
    return true;
}

int32_t WorkSchedulerService::StartWork(const WorkInfo& workInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StartWork");
    int32_t timerId = SetTimer();
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t ret = StartWorkInner(workInfo, uid);
    CancelTimer(timerId);
    return ret;
}

int32_t WorkSchedulerService::StartWorkInner(const WorkInfo& workInfo, int32_t uid)
{
    WorkInfo workInfo_ = workInfo;
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    if (checkBundle_ && !CheckWorkInfo(workInfo_, uid)) {
        WS_HILOGE("check workInfo failed, bundleName inconsistency.");
        return E_CHECK_WORKINFO_FAILED;
    }
    if (!CheckCondition(workInfo_)) {
        return E_REPEAT_CYCLE_TIME_ERR;
    }
    time_t baseTime;
    (void)time(&baseTime);
    workInfo_.RequestBaseTime(baseTime);
    WS_HILOGD("workInfo %{public}s/%{public}s ID: %{public}d, uid: %{public}d",
        workInfo_.GetBundleName().c_str(), workInfo_.GetAbilityName().c_str(), workInfo_.GetWorkId(), uid);
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo_, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uid);
    if (ret == ERR_OK) {
        workQueueManager_->AddWork(workStatus);
        if (workInfo_.IsPersisted()) {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            workStatus->workInfo_->RefreshUid(uid);
            persistedMap_.emplace(workStatus->workId_, workStatus->workInfo_);
            RefreshPersistedWorks();
        }
        GetHandler()->RemoveEvent(WorkEventHandler::CHECK_CONDITION_MSG);
        GetHandler()->SendEvent(InnerEvent::Get(WorkEventHandler::CHECK_CONDITION_MSG, 0),
            CHECK_CONDITION_DELAY);
    }
    return ret;
}

void WorkSchedulerService::AddWorkInner(WorkInfo& workInfo)
{
    WS_HILOGD("come in");
    if (workInfo.GetUid() > 0) {
        shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, workInfo.GetUid());
        if (workPolicyManager_->AddWork(workStatus, workInfo.GetUid()) == ERR_OK) {
            workQueueManager_->AddWork(workStatus);
        }
    } else {
        WS_HILOGE("uid is invalid : %{public}d", workInfo.GetUid());
    }
}

int32_t WorkSchedulerService::StartWorkForInner(const WorkInfo& workInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StartWorkForInner");
    if (!CheckCallingToken() || !CheckCallingServiceName()) {
        WS_HILOGE("StartWorkForInner not allowed.");
        return E_PERMISSION_DENIED;
    }
    WorkInfo workInfo_ = workInfo;
    workInfo_.SetIsInnerApply(true);
    int32_t timerId = SetTimer();
    int32_t uid;
    if (!GetUidByBundleName(workInfo_.GetBundleName(), uid)) {
        return E_INVALID_PROCESS_NAME;
    }
    int32_t ret = StartWorkInner(workInfo, uid);
    CancelTimer(timerId);
    return ret;
}

int32_t WorkSchedulerService::StopWork(const WorkInfo& workInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StopWork");
    WorkInfo workInfo_ = workInfo;
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo_, uid)) {
        WS_HILOGE("check workInfo failed, bundleName inconsistency.");
        return E_CHECK_WORKINFO_FAILED;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo_, uid);
    if (workStatus == nullptr) {
        WS_HILOGE("workStatus is nullptr");
        return E_WORK_NOT_EXIST_FAILED;
    }
    WS_HILOGI("StopWork %{public}s workId:%{public}d", workInfo_.GetBundleName().c_str(), workInfo_.GetWorkId());
    StopWorkInner(workStatus, uid, false, false);
    return ERR_OK;
}

int32_t WorkSchedulerService::StopWorkForInner(const WorkInfo& workInfo, bool needCancel)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StopWorkForInner");
    if (!CheckCallingToken() || !CheckCallingServiceName()) {
        WS_HILOGE("StopWorkForInner not allowed.");
        return E_PERMISSION_DENIED;
    }
    WorkInfo workInfo_ = workInfo;
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid;
    if (!GetUidByBundleName(workInfo_.GetBundleName(), uid)) {
        return E_INVALID_PROCESS_NAME;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo_, uid);
    if (workStatus == nullptr) {
        WS_HILOGE("workStatus is nullptr");
        return E_WORK_NOT_EXIST_FAILED;
    }
    WS_HILOGI("StopWorkForInner %{public}s workId:%{public}d",
        workInfo_.GetBundleName().c_str(), workInfo_.GetWorkId());
    StopWorkInner(workStatus, uid, needCancel, false);
    return ERR_OK;
}

int32_t WorkSchedulerService::StopAndCancelWork(const WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    WorkInfo workInfo_ = workInfo;
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo_, uid)) {
        WS_HILOGE("check workInfo failed, bundleName inconsistency.");
        return E_CHECK_WORKINFO_FAILED;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo_, uid);
    if (workStatus == nullptr) {
        WS_HILOGE("workStatus is nullptr");
        return E_WORK_NOT_EXIST_FAILED;
    }
    StopWorkInner(workStatus, uid, true, false);
    if (workStatus->persisted_) {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        persistedMap_.erase(workStatus->workId_);
        RefreshPersistedWorks();
    }
    WS_HILOGI("StopAndCancelWork %{public}s workId:%{public}d",
        workInfo_.GetBundleName().c_str(), workInfo_.GetWorkId());
    return ERR_OK;
}

bool WorkSchedulerService::StopWorkInner(std::shared_ptr<WorkStatus> workStatus, int32_t uid,
    const bool needCancel, bool isTimeOut)
{
    if (workPolicyManager_->StopWork(workStatus, uid, needCancel, isTimeOut)) {
        workQueueManager_->CancelWork(workStatus);
    }
    if (!isTimeOut) {
        workPolicyManager_->RemoveWatchDog(workStatus);
    }
    return true;
}

void WorkSchedulerService::WatchdogTimeOut(std::shared_ptr<WorkStatus> workStatus)
{
    StopWorkInner(workStatus, workStatus->uid_, false, true);
}

int32_t WorkSchedulerService::StopAndClearWorks()
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StopAndClearWorks");
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    StopAndClearWorksByUid(IPCSkeleton::GetCallingUid());
    return ERR_OK;
}

bool WorkSchedulerService::StopAndClearWorksByUid(int32_t uid)
{
    WS_HILOGD("Stop and clear works by Uid:%{public}d", uid);
    list<std::shared_ptr<WorkStatus>> allWorks = workPolicyManager_->GetAllWorkStatus(uid);
    list<std::string> workIdList;
    std::transform(allWorks.cbegin(), allWorks.cend(), std::back_inserter(workIdList),
        [](std::shared_ptr<WorkStatus> work) { return work->workId_; });
    bool ret = workQueueManager_->StopAndClearWorks(allWorks)
        && workPolicyManager_->StopAndClearWorks(uid);
    if (ret) {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        for (auto workId : workIdList) {
            if (persistedMap_.count(workId) != 0) {
                persistedMap_.erase(workId);
            }
        }
        RefreshPersistedWorks();
    }
    return ret;
}

int32_t WorkSchedulerService::IsLastWorkTimeout(int32_t workId, bool &result)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::IsLastWorkTimeout");
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    return workPolicyManager_->IsLastWorkTimeout(workId, uid, result);
}

void WorkSchedulerService::OnConditionReady(shared_ptr<vector<shared_ptr<WorkStatus>>> workStatusVector)
{
    workPolicyManager_->OnConditionReady(workStatusVector);
}

int32_t WorkSchedulerService::ObtainAllWorks(std::vector<WorkInfo>& workInfos)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::ObtainAllWorks");
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    workInfos = workPolicyManager_->ObtainAllWorks(uid);
    return ERR_OK;
}

int32_t WorkSchedulerService::ObtainWorksByUidAndWorkIdForInner(int32_t uid,
    std::vector<WorkInfo>& workInfos, int32_t workId)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::ObtainWorksByUidAndWorkIdForInner");
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    if (!CheckCallingToken()) {
        WS_HILOGE("ObtainWorksByUidAndWorkIdForInner not allowed.");
        return E_PERMISSION_DENIED;
    }
    if (workId != -1) {
        std::shared_ptr<WorkInfo> workInfoPtr = workPolicyManager_->GetWorkStatus(uid, workId);
        if (workInfoPtr != nullptr) {
            workInfos.push_back(*workInfoPtr);
            return ERR_OK;
        } else {
            WS_HILOGE("uid: %{public}d, workId: %{public}d no have task.", uid, workId);
            return E_WORK_NOT_EXIST_FAILED;
        }
    }
    workInfos = workPolicyManager_->ObtainAllWorks(uid);
    return ERR_OK;
}

int32_t WorkSchedulerService::GetWorkStatus(int32_t workId, WorkInfo& workInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::GetWorkStatus");
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    std::shared_ptr<WorkInfo> workInfoPtr = workPolicyManager_->GetWorkStatus(uid, workId);
    if (workInfoPtr != nullptr) {
        workInfo = *workInfoPtr;
        return ERR_OK;
    }
    return E_WORK_NOT_EXIST_FAILED;
}

int32_t WorkSchedulerService::GetAllRunningWorks(std::vector<WorkInfo>& workInfos)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    if (!CheckProcessName()) {
        return E_INVALID_PROCESS_NAME;
    }
    workInfos = workPolicyManager_->GetAllRunningWorks();
    return ERR_OK;
}

void WorkSchedulerService::UpdateWorkBeforeRealStart(std::shared_ptr<WorkStatus> work)
{
    if (work == nullptr) {
        return;
    }
    work->UpdateTimerIfNeed();
    if (work->NeedRemove()) {
        workQueueManager_->RemoveWork(work);
        if (work->persisted_ && !work->IsRepeating()) {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            persistedMap_.erase(work->workId_);
            RefreshPersistedWorks();
        }
    }
}

bool WorkSchedulerService::AllowDump()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetFirstTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, "ohos.permission.DUMP");
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        WS_HILOGE("CheckPermission failed");
        return false;
    }
    return true;
}

void WorkSchedulerService::DumpProcessForEngMode(std::vector<std::string> &argsInStr, std::string &result)
{
    switch (argsInStr.size()) {
        case 0:
            // hidumper -s said '-h'
            DumpUsage(result);
            break;
        case DUMP_OPTION + 1:
            // hidumper -s said '-h' or hidumper -s said '-a'
            if (argsInStr[DUMP_OPTION] == "-h") {
                DumpUsage(result);
            } else if (argsInStr[DUMP_OPTION] == "-a") {
                DumpAllInfo(result);
            } else {
                result.append("Error params.");
            }
            break;
        case DUMP_PARAM_INDEX + 1:
            if (argsInStr[DUMP_OPTION] == "-k") {
                string key = argsInStr[DUMP_PARAM_INDEX];
                string value;
                WorkDatashareHelper::GetInstance().GetStringValue(key, value);
                result.append("key: " + key + ", value: " + value);
                break;
            }
            DumpParamSet(argsInStr[DUMP_OPTION], argsInStr[DUMP_PARAM_INDEX], result);
            break;
        case DUMP_VALUE_INDEX + 1:
            if (argsInStr[DUMP_OPTION] == "-d") {
                EventPublisher eventPublisher;
                eventPublisher.Dump(result, argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX]);
            } else if (argsInStr[DUMP_OPTION] == "-t") {
                DumpProcessWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else if (argsInStr[DUMP_OPTION] == "-x") {
                DumpRunningWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else if (argsInStr[DUMP_OPTION] == "-s") {
                DumpLoadSaWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else if (argsInStr[DUMP_OPTION] == "-g") {
                DumpGetWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else if (argsInStr[DUMP_OPTION] == "-f") {
                DumpTriggerWork(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else {
                result.append("Error params.");
            }
            break;
        default:
            result.append("Error params.");
    }
}

int32_t WorkSchedulerService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    if (!AllowDump()) {
        return ERR_OK;
    }
    std::string result;
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        result.append("service is not ready.");
        if (!SaveStringToFd(fd, result)) {
            WS_HILOGE("save to fd failed.");
        }
        return ERR_OK;
    }

    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });
    bool secureMode = OHOS::system::GetBoolParameter("const.security.developermode.state", false);
    bool debugable = OHOS::system::GetIntParameter("const.debuggable", 0) == 1;
    if (secureMode && !debugable) {
        WS_HILOGD("User mode.");
        DumpProcessForUserMode(argsInStr, result);
    } else if (debugable) {
        WS_HILOGD("Eng mode.");
        DumpProcessForEngMode(argsInStr, result);
    }
    if (!SaveStringToFd(fd, result)) {
        WS_HILOGE("save to fd failed.");
    }
    return ERR_OK;
}

void WorkSchedulerService::DumpProcessForUserMode(std::vector<std::string> &argsInStr, std::string &result)
{
    if (argsInStr.size() == (DUMP_VALUE_INDEX + 1) && argsInStr[DUMP_OPTION] == "-t") {
        DumpProcessWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
    } else if (argsInStr.size() == (DUMP_VALUE_INDEX + 1) && argsInStr[DUMP_OPTION] == "-s") {
        DumpLoadSaWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
    }
}

void WorkSchedulerService::DumpUsage(std::string &result)
{
    result.append("usage: workscheduler dump [<options>]\n")
        .append("    -h: show the help.\n")
        .append("    -a: show all info.\n")
        .append("    -d event info: show the event info.\n")
        .append("    -d (eventType) (TypeValue): publish the event.\n")
        .append("    -t (bundleName) (abilityName): trigger the bundleName all works.\n")
        .append("    -f (uId) (workId): trigger the work.\n")
        .append("    -x (uid) (option): pause or resume the work.\n")
        .append("    -memory (number): set the available memory.\n")
        .append("    -watchdog_time (number): set watch dog time, default 120000.\n")
        .append("    -repeat_time_min (number): set min repeat cycle time, default 1200000.\n")
        .append("    -min_interval (number): set min interval time, set 0 means close test mode.\n")
        .append("    -cpu (number): set the usage cpu.\n")
        .append("    -count (number): set the max running task count.\n")
        .append("    -s (saId) (uId): load or report sa.\n");
}

void WorkSchedulerService::DumpAllInfo(std::string &result)
{
    result.append("================Work Queue Infos================\n");
    if (workQueueManager_ != nullptr) {
        workQueueManager_->Dump(result);
    }
    result.append("================Work Policy Infos================\n");
    if (workPolicyManager_ != nullptr) {
        workPolicyManager_->Dump(result);
    }
    result.append("================Other Infos================\n");
    result.append("Need check bundle:" + std::to_string(checkBundle_) + "\n")
        .append("Dump set memory:" + std::to_string(workPolicyManager_->GetDumpSetMemory()) + "\n")
        .append("Repeat cycle time min:" + std::to_string(workQueueManager_->GetTimeCycle()) + "\n")
        .append("Watchdog time:" + std::to_string(workPolicyManager_->GetWatchdogTime()) + "\n")
        .append("Exemption bundle whitelist:" + DumpExemptionBundles() + "\n")
        .append("Efficiency Resource whitelist:" + DumpEffiResApplyUid() + "\n");
}

bool WorkSchedulerService::IsDebugApp(const std::string &bundleName)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::WORK_CHECK, "fail to get system ability manager");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager proxy.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::WORK_CHECK, "fail to get bundle manager proxy");
        return false;
    }
    sptr<IBundleMgr> bundleMgr =  iface_cast<IBundleMgr>(remoteObject);
    BundleInfo bundleInfo;
    int32_t currentAccountId = WorkSchedUtils::GetCurrentAccountId();
    if (bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, currentAccountId)) {
        WS_HILOGD("bundleUid : %{public}d , debug : %{public}d.", bundleInfo.uid, bundleInfo.applicationInfo.debug);
        return bundleInfo.applicationInfo.debug;
    }
    WS_HILOGE("Get bundle info failed.");
    WorkSchedUtil::HiSysEventException(EventErrorCode::WORK_CHECK, "fail to get bundle info");
    return false;
}

void WorkSchedulerService::DumpProcessWorks(const std::string &bundleName, const std::string &abilityName,
    std::string &result)
{
    if (bundleName.empty() || abilityName.empty()) {
        result.append("param error");
        return;
    }
    workPolicyManager_->DumpCheckIdeWorkToRun(bundleName, abilityName);
}

void WorkSchedulerService::DumpTriggerWork(const std::string& uIdStr, const std::string& workIdStr, std::string& result)
{
    if (uIdStr.empty() || workIdStr.empty() || !std::all_of(uIdStr.begin(), uIdStr.end(), ::isdigit)
        || !std::all_of(workIdStr.begin(), workIdStr.end(), ::isdigit)) {
        result.append("param invalid\n");
        return;
    }
    int32_t uId = std::atoi(uIdStr.c_str());
    if (uId <= 0) {
        result.append("uIdStr param invalid, uIdStr:" + uIdStr + "\n");
        return;
    }
    int32_t workId = std::atoi(workIdStr.c_str());
    if (workId <= 0) {
        result.append("workIdStr param invalid, workIdStr:" + workIdStr + "\n");
        return;
    }
    workPolicyManager_->DumpTriggerWork(uId, workId, result);
}

void WorkSchedulerService::DumpRunningWorks(const std::string &uidStr, const std::string &option, std::string &result)
{
    if (!std::all_of(uidStr.begin(), uidStr.end(), ::isdigit) || option.empty()) {
        result.append("param error");
        return;
    }

    int32_t uid = std::atoi(uidStr.c_str());
    if (uid == 0) {
        result.append("uidStr param error, uidStr:" + uidStr);
        return;
    }
    int32_t ret = ERR_OK;
    if (option == "p") {
        ret = workPolicyManager_->PauseRunningWorks(uid);
    } else if (option == "r") {
        ret = workPolicyManager_->ResumePausedWorks(uid);
    } else {
        result.append("param error");
    }

    if (ret != ERR_OK) {
        auto iter = paramErrCodeMsgMap.find(ret);
        if (iter != paramErrCodeMsgMap.end()) {
            result.append("BussinessError:" + iter->second);
        }
    }
}

std::string WorkSchedulerService::DumpEffiResApplyUid()
{
    std::lock_guard<ffrt::mutex> lock(whitelistMutex_);
    if (whitelist_.empty()) {
        return "[]";
    }
    std::string res {""};
    for (auto &it : whitelist_) {
        res.append(std::to_string(it) + " ");
    }
    WS_HILOGD("GetWhiteList  : %{public}s", res.c_str());
    return res;
}

std::string WorkSchedulerService::DumpExemptionBundles()
{
    if (exemptionBundles_.empty()) {
        return "[]";
    }

    std::string bundles {""};
    for (auto &bundle : exemptionBundles_) {
        bundles.append(bundle + " ");
    }
    return bundles;
}

void WorkSchedulerService::DumpParamSet(std::string &key, std::string &value, std::string &result)
{
    if (!std::all_of(value.begin(), value.end(), ::isdigit)) {
        result.append("Error params.");
        return;
    }
    if (key == "-memory") {
        workPolicyManager_->SetMemoryByDump(std::stoi(value));
        result.append("Set memory success.");
    } else if (key == "-watchdog_time") {
        workPolicyManager_->SetWatchdogTimeByDump(std::stoi(value));
        result.append("Set watchdog time success.");
    } else if (key == "-repeat_time_min") {
        workQueueManager_->SetTimeCycle(std::stoi(value));
        result.append("Set repeat time min value success.");
    } else if (key == "-min_interval") {
        workQueueManager_->SetMinIntervalByDump(std::stoi(value));
        result.append("Set min interval value success.");
    } else if (key == "-cpu") {
        workPolicyManager_->SetCpuUsageByDump(std::stoi(value));
        result.append("Set cpu success.");
    } else if (key == "-nap") {
#ifdef DEVICE_STANDBY_ENABLE
        standbyStateObserver_->OnDeviceIdleMode(std::stoi(value), 0);
#endif
    } else if (key == "-count") {
        workPolicyManager_->SetMaxRunningCountByDump(std::stoi(value));
        result.append("Set max running task count success.");
    } else {
        result.append("Error params.");
    }
}

void WorkSchedulerService::RefreshPersistedWorks()
{
    nlohmann::json root;
    for (auto &it : persistedMap_) {
        if (it.second == nullptr) {
            WS_HILOGE("workInfo is nullptr");
            continue;
        }
        auto workInfo = it.second;
        string data = workInfo->ParseToJsonStr();
        const nlohmann::json &workJson = nlohmann::json::parse(data, nullptr, false);
        if (!workJson.is_discarded()) {
            root[it.first] = workJson;
        }
    }
    string result = root.dump(4);
    CreateNodeDir(PERSISTED_PATH);
    CreateNodeFile();
    ofstream fout;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(PERSISTED_FILE_PATH, realPath)) {
        WS_HILOGE("Get real path failed");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "get real path failed");
        return;
    }
    WS_HILOGD("Refresh path %{private}s", realPath.c_str());
    fout.open(realPath, ios::out);
    fout<<result.c_str()<<endl;
    fout.close();
    ReportUserDataSizeEvent();
    WS_HILOGD("Refresh persisted works success");
}

int32_t WorkSchedulerService::CreateNodeDir(std::string dir)
{
    WS_HILOGD("Enter");
    if (access(dir.c_str(), 0) != ERR_OK) {
        int32_t flag = mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
        if (flag == ERR_OK) {
            WS_HILOGD("Create directory successfully.");
        } else {
            WS_HILOGE("Fail to create directory, flag: %{public}d", flag);
            WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "fail to create directory");
            return flag;
        }
    } else {
        WS_HILOGD("This directory already exists.");
    }
    return ERR_OK;
}

int32_t WorkSchedulerService::CreateNodeFile()
{
    if (access(PERSISTED_FILE_PATH, 0) != 0) {
        FILE *file = fopen(PERSISTED_FILE_PATH, "w+");
        if (file == nullptr) {
            WS_HILOGE("Fail to open file: %{private}s, errno: %{public}s", PERSISTED_FILE_PATH, strerror(errno));
            WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "fail to open file");
            return errno;
        }
        WS_HILOGI("Open file success.");
        int closeResult = fclose(file);
        if (closeResult < 0) {
            WS_HILOGE("Fail to close file: %{private}s, errno: %{public}s", PERSISTED_FILE_PATH, strerror(errno));
            WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "fail to close file");
            return errno;
        }
    } else {
        WS_HILOGD("The file already exists.");
    }
    return ERR_OK;
}

void WorkSchedulerService::UpdateEffiResApplyInfo(int32_t uid, bool isAdd)
{
    std::lock_guard<ffrt::mutex> lock(whitelistMutex_);
    if (isAdd) {
        whitelist_.emplace(uid);
    } else {
        whitelist_.erase(uid);
    }
}

bool WorkSchedulerService::CheckEffiResApplyInfo(int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(whitelistMutex_);
    return whitelist_.find(uid) != whitelist_.end();
}

void WorkSchedulerService::InitDeviceStandyWhitelist()
{
#ifdef DEVICE_STANDBY_ENABLE
    std::vector<DevStandbyMgr::AllowInfo> allowInfoArray;
    auto res = DevStandbyMgr::StandbyServiceClient::GetInstance().GetAllowList(DevStandbyMgr::AllowType::WORK_SCHEDULER,
        allowInfoArray, DevStandbyMgr::ReasonCodeEnum::REASON_APP_API);
    if (res != ERR_OK) {
        WS_HILOGE("GetAllowList fail");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "get device standby white list failed");
        return;
    }
    WS_HILOGI("allowInfoArray size is %{public}d", static_cast<int32_t>(allowInfoArray.size()));
    std::list<std::string> tempList = {};
    for (const auto& item : allowInfoArray) {
        WS_HILOGI("Allow bundleName %{public}s", item.GetName().c_str());
        tempList.push_back(item.GetName());
    }
    DelayedSingleton<DataManager>::GetInstance()->AddDeviceStandyWhitelist(tempList);
#endif
}

void WorkSchedulerService::InitDeviceStandyRestrictlist()
{
#ifdef DEVICE_STANDBY_ENABLE
    std::vector<DevStandbyMgr::AllowInfo> allowInfoArray;
    auto res = DevStandbyMgr::StandbyServiceClient::GetInstance().GetRestrictList(
        DevStandbyMgr::AllowType::WORK_SCHEDULER, allowInfoArray, DevStandbyMgr::ReasonCodeEnum::REASON_APP_API);
    if (res != ERR_OK) {
        WS_HILOGE("GetRestrictlist fail");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "get device standby restrict list failed");
        return;
    }
    WS_HILOGI("restrictInfoArray size is %{public}d", static_cast<int32_t>(allowInfoArray.size()));
    std::list<std::string> tempList = {};
    for (const auto& item : allowInfoArray) {
        WS_HILOGI("Restrict bundleName %{public}s", item.GetName().c_str());
        tempList.push_back(item.GetName());
    }
    DelayedSingleton<DataManager>::GetInstance()->AddDeviceStandyRestrictlist(tempList);
#endif
}

void WorkSchedulerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId == DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID) {
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
        GroupObserverInit();
#endif
    }
    if (systemAbilityId == DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID) {
        InitDeviceStandyWhitelist();
        InitDeviceStandyRestrictlist();
        RegisterStandbyStateObserver();
    }
}

void WorkSchedulerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId == DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID) {
        DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyWhitelist();
        DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyRestrictlist();
        if (!workQueueManager_) {
            return;
        }
        workQueueManager_->OnConditionChanged(WorkCondition::Type::STANDBY,
            std::make_shared<DetectorValue>(0, 0, false, std::string()));
#ifdef  DEVICE_STANDBY_ENABLE
        std::lock_guard<ffrt::mutex> observerLock(observerMutex_);
        standbyStateObserver_ = nullptr;
#endif
    } else if (systemAbilityId == DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID) {
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
        std::lock_guard<ffrt::mutex> observerLock(observerMutex_);
        groupObserver_ = nullptr;
        DelayedSingleton<DataManager>::GetInstance()->ClearAllGroup();
#endif
    }
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
__attribute__((no_sanitize("cfi"))) void WorkSchedulerService::GroupObserverInit()
{
    if (!workQueueManager_) {
        return;
    }
    std::lock_guard<ffrt::mutex> observerLock(observerMutex_);
    if (!groupObserver_) {
        groupObserver_ = new (std::nothrow) WorkBundleGroupChangeCallback(workQueueManager_);
    }
    if (groupObserver_ && g_hasGroupObserver != ERR_OK) {
        g_hasGroupObserver =
            DeviceUsageStats::BundleActiveClient::GetInstance().RegisterAppGroupCallBack(groupObserver_);
    }
}
#endif

void WorkSchedulerService::RegisterStandbyStateObserver()
{
    if (!workQueueManager_) {
        return;
    }
#ifdef  DEVICE_STANDBY_ENABLE
    std::lock_guard<ffrt::mutex> observerLock(observerMutex_);
    if (standbyStateObserver_) {
        WS_HILOGD("standbyStateObserver_ is already exist, do not need repeat process.");
        return;
    }
    standbyStateObserver_ = new (std::nothrow) WorkStandbyStateChangeCallback(workQueueManager_);
    if (!standbyStateObserver_) {
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "create standby state observer failed");
        return;
    }
    standbyStateObserver_->SetSubscriberName(STRATEGY_NAME);
    ErrCode ret = DevStandbyMgr::StandbyServiceClient::GetInstance().SubscribeStandbyCallback(standbyStateObserver_);
    if (ret != ERR_OK) {
        WS_HILOGE("Subscriber standbyStateObserver_ failed.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::SERVICE_INIT, "subscribe standby state observer failed");
        standbyStateObserver_ = nullptr;
    }
#endif
}

bool WorkSchedulerService::CheckProcessName()
{
    Security::AccessToken::AccessTokenID tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo callingTokenInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, callingTokenInfo);
    WS_HILOGD("process name: %{public}s called CheckProcessName.", callingTokenInfo.processName.c_str());
    if (WORK_SCHED_NATIVE_OPERATE_CALLER.find(callingTokenInfo.processName) == WORK_SCHED_NATIVE_OPERATE_CALLER.end()) {
        WS_HILOGE("check process name illegal access to this interface; process name: %{public}s.",
            callingTokenInfo.processName.c_str());
        WorkSchedUtil::HiSysEventException(EventErrorCode::TOKEN_CHECK, "illegal process name");
        return false;
    }
    return true;
}

bool WorkSchedulerService::CheckCallingServiceName()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo callingTokenInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, callingTokenInfo);
    WS_HILOGD("process name: %{public}s called CheckCallingServiceName.", callingTokenInfo.processName.c_str());
    if (WORK_SCHED_SA_CALLER.find(callingTokenInfo.processName) == WORK_SCHED_SA_CALLER.end()) {
        WS_HILOGE("check process name illegal, process name: %{public}s.", callingTokenInfo.processName.c_str());
        return false;
    }
    return true;
}

int32_t WorkSchedulerService::PauseRunningWorks(int32_t uid)
{
    WS_HILOGD("pause Running Work Scheduler Work, uid:%{public}d", uid);
    if (!CheckProcessName() || !CheckCallingToken()) {
        return E_INVALID_PROCESS_NAME;
    }

    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    return ret;
}

int32_t WorkSchedulerService::ResumePausedWorks(int32_t uid)
{
    WS_HILOGD("resume Paused Work Scheduler Work, uid:%{public}d", uid);
    if (!CheckProcessName() || !CheckCallingToken()) {
        return E_INVALID_PROCESS_NAME;
    }

    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    return ret;
}

void WorkSchedulerService::TriggerWorkIfConditionReady()
{
    ConditionChecker checker(workQueueManager_);
    checker.CheckAllStatus();
}

int32_t WorkSchedulerService::StopDeepIdleWorks()
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    std::list<std::shared_ptr<WorkStatus>> works =  workPolicyManager_->GetDeepIdleWorks();
    if (works.size() == 0) {
        WS_HILOGD("stop work by condition, no matched works");
        return ERR_OK;
    }

    for (shared_ptr<WorkStatus> workStatus : works) {
        WS_HILOGI("stop work by condition, bundleName:%{public}s, workId:%{public}s",
            workStatus->bundleName_.c_str(), workStatus->workId_.c_str());
        StopWorkInner(workStatus, workStatus->uid_, false, false);
        workPolicyManager_->RemoveWatchDog(workStatus);
    }
    return ERR_OK;
}

int32_t WorkSchedulerService::SetWorkSchedulerConfig(const std::string &configData, int32_t sourceType)
{
    if (!ready_) {
        WS_HILOGE("service is not ready");
        return E_SERVICE_NOT_READY;
    }
    if (!CheckProcessName() || !CheckCallingToken()) {
        return E_INVALID_PROCESS_NAME;
    }
    WS_HILOGD("Set work scheduler configData: %{public}s, sourceType: %{public}d", configData.c_str(), sourceType);
    DelayedSingleton<WorkSchedulerConfig>::GetInstance()->InitActiveGroupWhitelist(configData);
    return ERR_OK;
}

bool WorkSchedulerService::IsExemptionBundle(const std::string& checkBundleName)
{
    if (checkBundleName.empty()) {
        WS_HILOGE("check exemption bundle error, bundleName is empty");
        return false;
    }
    auto iter = std::find_if(exemptionBundles_.begin(), exemptionBundles_.end(),
    [&](const std::string &bundleName) {
        return checkBundleName == bundleName;
    });
    return iter != exemptionBundles_.end();
}

bool WorkSchedulerService::LoadSa(std::shared_ptr<WorkStatus> workStatus, const std::string& action)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return false;
    }
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        WS_HILOGE("get SA manager failed.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::LOAD_SA, "get system ability manager failed");
        return false;
    }
    int32_t saId = workStatus->workInfo_->GetSaId();
    bool isResidentSa = workStatus->workInfo_->IsResidentSa();
    sptr<IRemoteObject> object = samgr->CheckSystemAbility(saId);
    if (isResidentSa && object == nullptr) {
        WS_HILOGE("resident SA: %{public}d residentSA:%{public}d does not exist.", saId, isResidentSa);
        WorkSchedUtil::HiSysEventException(EventErrorCode::LOAD_SA, "sa dose not exist");
        return false;
    } else if (!isResidentSa && object == nullptr) {
        object = samgr->LoadSystemAbility(saId, TIME_OUT);
        if (object == nullptr) {
            WS_HILOGE("load SA: %{public}d residentSA:%{public}d failed.", saId, isResidentSa);
            WorkSchedUtil::HiSysEventException(EventErrorCode::LOAD_SA, "load system ability failed");
            return false;
        }
        WS_HILOGI("load SA: %{public}d residentSA:%{public}d successed.", saId, isResidentSa);
    }
    std::unordered_map<std::string, std::string> payload;
    payload["action"] = action;
    payload["saId"] = std::to_string(saId);
    uint32_t type = ResourceSchedule::ResType::RES_TYPE_DEVICE_IDLE;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
    WS_HILOGI("Report SA: %{public}d residentSA:%{public}d successed.", saId, isResidentSa);
    return true;
}

void WorkSchedulerService::DumpLoadSaWorks(const std::string &saIdStr, const std::string &uidStr, std::string &result)
{
    if (!std::all_of(saIdStr.begin(), saIdStr.end(), ::isdigit) ||
        !std::all_of(uidStr.begin(), uidStr.end(), ::isdigit)) {
        result.append("param error.");
        return;
    }
    int32_t saId = std::atoi(saIdStr.c_str());
    int32_t uid = std::atoi(uidStr.c_str());
    if (saId <= 0 || uid <= 0) {
        result.append("the parameter is invalid.");
        return;
    }
    auto sa = workPolicyManager_->FindSA(saId, uid);
    if (sa == nullptr) {
        result.append("the sa does not exist.");
        return;
    }
    if (LoadSa(sa, "hidumper")) {
        result.append("load sa success.");
        return;
    }
    result.append("load sa failed.");
}

void WorkSchedulerService::DumpGetWorks(const std::string &uidStr, const std::string &workIdStr, std::string &result)
{
    if (workIdStr.empty() || uidStr.empty()) {
        result.append("param error.");
        return;
    }
    int32_t workId = std::atoi(workIdStr.c_str());
    int32_t uid = std::atoi(uidStr.c_str());
    if (uid < 0) {
        result.append("the parameter is invalid.");
        return;
    }
    std::vector<WorkInfo> workInfos;
    int32_t ret = ObtainWorksByUidAndWorkIdForInner(uid, workInfos, workId);
    if (ret != ERR_OK) {
        result.append("get work fail.");
        return;
    }
    if (workInfos.size() == 0) {
        result.append("no have workscheduler task.");
        return;
    }
    for (auto &info : workInfos) {
        info.Dump(result);
    }
}

void WorkSchedulerService::HandleDeepIdleMsg()
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return;
    }
    workQueueManager_->OnConditionChanged(WorkCondition::Type::DEEP_IDLE,
        std::make_shared<DetectorValue>(0, 0, true, std::string()));
    WorkSchedUtil::HiSysEventStateChanged({"DEEP_IDLE", 1});
}

bool WorkSchedulerService::IsPreinstalledBundle(const std::string& checkBundleName)
{
    if (checkBundleName.empty()) {
        WS_HILOGE("check preinstalled bundle error, bundleName is empty");
        return false;
    }
    return preinstalledBundles_.find(checkBundleName) != preinstalledBundles_.end();
}

int32_t WorkSchedulerService::StopWorkForSA(int32_t saId)
{
    WS_HILOGI("StopWork for SA:%{public}d success", saId);
    return ERR_OK;
}

int32_t WorkSchedulerService::SetTimer()
{
#ifdef HICOLLIE_ENABLE
    int32_t idTimer = HiviewDFX::INVALID_ID;
    std::string collieName = "WorkSchedulerService:START_WORK";
    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_RECOVERY;
    auto TimerCallback = [](void *) {
        WS_HILOGE("OnRemoteRequest timeout func: START_WORK");
    };
    idTimer = HiviewDFX::XCollie::GetInstance().SetTimer(
        collieName, XCOLLIE_TIMEOUT_SECONDS, TimerCallback, nullptr, flag);
    WS_HILOGI("SetTimer id: %{public}d, name: %{public}s.", idTimer, collieName.c_str());
    return idTimer;
#else
    WS_HILOGD("No HICOLLIE_ENABLE");
    return -1;
#endif
}

void WorkSchedulerService::CancelTimer(int32_t id)
{
#ifdef HICOLLIE_ENABLE
    if (id == HiviewDFX::INVALID_ID) {
        return;
    }
    WS_HILOGI("CancelTimer id: %{public}d.", id);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
#else
    return;
#endif
}

bool WorkSchedulerService::CheckCallingToken()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }
    return false;
}

uint64_t GetRemainPartitionSize(const std::string& partitionName)
{
    struct statfs stat;
    if (statfs(partitionName.c_str(), &stat) != 0) {
        return -1;
    }
    uint64_t blockSize = stat.f_bsize;
    uint64_t freeSize = stat.f_bfree * blockSize;
    constexpr double units = 1024.0;
    return freeSize / (units * units);
}

std::vector<uint64_t> GetFileOrFolderSize(const std::vector<std::string>& paths)
{
    std::vector<uint64_t> folderSize;
    for (auto path : paths) {
        folderSize.emplace_back(OHOS::GetFolderSize(path));
    }
    return folderSize;
}

void WorkSchedulerService::ReportUserDataSizeEvent()
{
    std::vector<std::string> paths = {
        "/data/service/el1/public/WorkScheduler/"
    };
    uint64_t remainPartitionSize = GetRemainPartitionSize("/data");
    std::vector<uint64_t> folderSize = GetFileOrFolderSize(paths);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, "USER_DATA_SIZE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "COMPONENT_NAME", "work_scheduler",
        "PARTITION_NAME", "/data",
        "REMAIN_PARTITION_SIZE", remainPartitionSize,
        "FILE_OR_FOLDER_PATH", paths,
        "FILE_OR_FOLDER_SIZE", folderSize);
}
} // namespace WorkScheduler
} // namespace OHOS
