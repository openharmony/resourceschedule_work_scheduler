/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "work_status.h"

#include "time_service_client.h"
#include "work_datashare_helper.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#include "bundle_active_group_map.h"
#endif
#include "parameters.h"
#include "work_sched_data_manager.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
static const double ONE_SECOND = 1000.0;
static bool groupDebugMode = false;
static const int64_t MIN_INTERVAL_DEFAULT = 2 * 60 * 60 * 1000;
std::map<int32_t, time_t> WorkStatus::s_uid_last_time_map;
const int32_t DEFAULT_PRIORITY = 10000;
const int32_t HIGH_PRIORITY = 0;
const int32_t ACTIVE_GROUP = 10;
const string SWITCH_ON = "1";
ffrt::mutex WorkStatus::s_uid_last_time_mutex;

time_t getCurrentTime()
{
    time_t result;
    time(&result);
    return result;
}

time_t getOppositeTime()
{
    time_t result;
    sptr<MiscServices::TimeServiceClient> timer = MiscServices::TimeServiceClient::GetInstance();
    result = static_cast<time_t>(timer->GetBootTimeMs());
    return result;
}

WorkStatus::WorkStatus(WorkInfo &workInfo, int32_t uid)
{
    this->workInfo_ = make_shared<WorkInfo>(workInfo);
    this->workId_ = MakeWorkId(workInfo.GetWorkId(), uid);
    this->bundleName_ = workInfo.GetBundleName();
    this->abilityName_ = workInfo.GetAbilityName();
    this->baseTime_ = workInfo.GetBaseTime();
    this->uid_ = uid;
    this->userId_ = WorkSchedUtils::GetUserIdByUid(uid);
    if (workInfo.GetConditionMap()->count(WorkCondition::Type::TIMER) > 0) {
        auto workTimerCondition = workInfo.GetConditionMap()->at(WorkCondition::Type::TIMER);
        shared_ptr<Condition> timeCondition = make_shared<Condition>();
        timeCondition->uintVal = workTimerCondition->uintVal;
        timeCondition->boolVal = workTimerCondition->boolVal;
        if (!workTimerCondition->boolVal) {
            timeCondition->intVal = workTimerCondition->intVal;
        }
        std::lock_guard<ffrt::mutex> lock(conditionMapMutex_);
        conditionMap_.emplace(WorkCondition::Type::TIMER, timeCondition);
    }
    this->persisted_ = workInfo.IsPersisted();
    this->priority_ = GetPriority();
    this->currentStatus_ = WAIT_CONDITION;
    this->minInterval_ = MIN_INTERVAL_DEFAULT;
    this->groupChanged_ = false;
}

WorkStatus::~WorkStatus() {}

int32_t WorkStatus::OnConditionChanged(WorkCondition::Type &type, shared_ptr<Condition> value)
{
    WS_HILOGD("Work status condition changed.");
    if (workInfo_->GetConditionMap()->count(type) > 0
        && type != WorkCondition::Type::TIMER
        && type != WorkCondition::Type::GROUP) {
        std::lock_guard<ffrt::mutex> lock(conditionMapMutex_);
        if (conditionMap_.count(type) > 0) {
            conditionMap_.at(type) = value;
        } else {
            conditionMap_.emplace(type, value);
        }
    }
    groupChanged_ = false;
    if (type == WorkCondition::Type::GROUP && value && value->boolVal) {
        WS_HILOGD("Group changed, bundleName: %{public}s.", value->strVal.c_str());
        groupChanged_ = true;
        if (value->intVal == userId_ && value->strVal == bundleName_) {
            SetMinIntervalByGroup(value->enumVal);
        } else {
            return E_GROUP_CHANGE_NOT_MATCH_HAP;
        }
    }
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    if (dataManager->GetDeviceSleep() && !dataManager->IsInDeviceStandyWhitelist(bundleName_)) {
        WS_HILOGI("Standby mode, Work status:%{public}s not standby exempted.", bundleName_.c_str());
        return E_GROUP_CHANGE_NOT_MATCH_HAP;
    }
    if (IsReady()) {
        MarkStatus(Status::CONDITION_READY);
    }
    return ERR_OK;
}

string WorkStatus::MakeWorkId(int32_t workId, int32_t uid)
{
    return string("u") + to_string(uid) + "_" + to_string(workId);
}

void WorkStatus::MarkTimeout()
{
    lastTimeout_ = true;
}

void WorkStatus::MarkStatus(Status status)
{
    currentStatus_ = status;
}

void WorkStatus::MarkRound() {}

void WorkStatus::UpdateTimerIfNeed()
{
    std::lock_guard<ffrt::mutex> lock(conditionMapMutex_);
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        baseTime_ = getCurrentTime();
        if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
            workInfo_->RequestBaseTime(baseTime_);
            DelayedSingleton<WorkSchedulerService>::GetInstance()->RefreshPersistedWorks();
            return;
        }
        int32_t cycleLeft = conditionMap_.at(WorkCondition::Type::TIMER)->intVal;
        conditionMap_.at(WorkCondition::Type::TIMER)->intVal = cycleLeft - 1;
        workInfo_->RequestBaseTimeAndCycle(baseTime_, cycleLeft - 1);
        DelayedSingleton<WorkSchedulerService>::GetInstance()->RefreshPersistedWorks();
    }
}

bool WorkStatus::NeedRemove()
{
    std::lock_guard<ffrt::mutex> lock(conditionMapMutex_);
    if (conditionMap_.count(WorkCondition::Type::TIMER) <= 0) {
        return true;
    }
    if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
        return false;
    }
    if (conditionMap_.at(WorkCondition::Type::TIMER)->intVal <= 0) {
        return true;
    }
    return false;
}

bool WorkStatus::IsSameUser()
{
    if (userId_ > 0 && !WorkSchedUtils::IsIdActive(userId_)) {
        return false;
    }
    return true;
}

bool WorkStatus::IsUriKeySwitchOn()
{
    if (!workInfo_->IsPreinstalled()) {
        return true;
    }
    if (workInfo_->GetUriKey().empty()) {
        WS_HILOGE("key is empty %{public}s", workId_.c_str());
        return false;
    }
    string key = workInfo_->GetUriKey();
    string value;
    (void)WorkDatashareHelper::GetInstance().GetStringValue(key, value);
    if (value == SWITCH_ON) {
        return true;
    }
    WS_HILOGE("workid %{public}s key %{public}s, value is 0", workId_.c_str(), key.c_str());
    return false;
}

bool WorkStatus::IsReady()
{
    WS_HILOGD("IsReady");
    if (!IsSameUser()) {
        WS_HILOGI("Not same user. WorkId:%{public}s", workId_.c_str());
        return false;
    }
    if (IsRunning()) {
        WS_HILOGD("Work is running");
        return false;
    }
    auto workConditionMap = workInfo_->GetConditionMap();
    std::lock_guard<ffrt::mutex> lock(s_uid_last_time_mutex);
    for (auto it : *workConditionMap) {
        if (conditionMap_.count(it.first) <= 0) {
            return false;
        }
        if (!IsBatteryAndNetworkReady(it.first) || !IsStorageAndTimerReady(it.first) ||
            !IsChargerReady(it.first) || !IsNapReady(it.first) || !IsUnLockReady()) {
            return false;
        }
    }
    if (!IsUriKeySwitchOn()) {
        return false;
    }
    if (DelayedSingleton<WorkSchedulerService>::GetInstance()->CheckEffiResApplyInfo(uid_)) {
        return true;
    }
    if (!groupDebugMode && ((!groupChanged_ && !SetMinInterval()) || minInterval_ == -1)) {
        WS_HILOGE("Work can't ready due to false group, forbidden group or unused group, bundleName:%{public}s, "
            "minInterval:%{public}" PRId64 ", workId:%{public}s", bundleName_.c_str(), minInterval_, workId_.c_str());
        return false;
    }

    auto itMap = s_uid_last_time_map.find(uid_);
    if (itMap == s_uid_last_time_map.end()) {
        WS_HILOGI("First trigger, bundleName:%{public}s, uid:%{public}d", bundleName_.c_str(), uid_);
        return true;
    }
    time_t lastTime = s_uid_last_time_map[uid_];
    double del = difftime(getOppositeTime(), lastTime);
    if (del < minInterval_) {
        WS_HILOGI("Condition not ready, bundleName:%{public}s, workId:%{public}s, "
            "minInterval:%{public}" PRId64 ", del:%{public}f", bundleName_.c_str(), workId_.c_str(), minInterval_, del);
        needRetrigger_ = true;
        timeRetrigger_ = int(minInterval_ - del + ONE_SECOND);
        return false;
    }
    WS_HILOGI("Condition Ready, bundleName:%{public}s, abilityName:%{public}s, workId:%{public}s, "
        "callbackFlag:%{public}d, minInterval:%{public}" PRId64 ", del = %{public}f",
        bundleName_.c_str(), abilityName_.c_str(), workId_.c_str(), groupChanged_, minInterval_, del);
    return true;
}

bool WorkStatus::IsBatteryAndNetworkReady(WorkCondition::Type type)
{
    auto workConditionMap = workInfo_->GetConditionMap();
    switch (type) {
        case WorkCondition::Type::NETWORK: {
            if (conditionMap_.at(type)->enumVal == WorkCondition::Network::NETWORK_UNKNOWN) {
                return false;
            }
            if (workConditionMap->at(type)->enumVal != WorkCondition::Network::NETWORK_TYPE_ANY &&
                workConditionMap->at(type)->enumVal != conditionMap_.at(type)->enumVal) {
                return false;
            }
            break;
        }
        case WorkCondition::Type::BATTERY_STATUS: {
            int32_t batteryReq = workConditionMap->at(type)->enumVal;
            if (batteryReq != WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY &&
                batteryReq != conditionMap_.at(type)->enumVal) {
                return false;
            }
            break;
        }
        case WorkCondition::Type::BATTERY_LEVEL: {
            if (workConditionMap->at(type)->intVal > conditionMap_.at(type)->intVal) {
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

bool WorkStatus::IsChargerReady(WorkCondition::Type type)
{
    if (type != WorkCondition::Type::CHARGER) {
        return true;
    }
    auto conditionSet = workInfo_->GetConditionMap()->at(WorkCondition::Type::CHARGER);
    auto conditionCurrent = conditionMap_.at(WorkCondition::Type::CHARGER);
    if (conditionSet->boolVal != conditionCurrent->boolVal) {
        return false;
    }
    if (conditionSet->boolVal) {
        if (conditionCurrent->enumVal != conditionSet->enumVal && conditionSet->enumVal !=
            static_cast<int32_t>(WorkCondition::Charger::CHARGING_PLUGGED_ANY)) {
            return false;
        }
    } else {
        if (conditionCurrent->enumVal != static_cast<int32_t>(WorkCondition::Charger::CHARGING_UNPLUGGED)) {
            return false;
        }
    }
    return true;
}


bool WorkStatus::IsStorageAndTimerReady(WorkCondition::Type type)
{
    auto workConditionMap = workInfo_->GetConditionMap();
    switch (type) {
        case WorkCondition::Type::STORAGE: {
            if (workConditionMap->at(type)->enumVal != WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY &&
                workConditionMap->at(type)->enumVal != conditionMap_.at(type)->enumVal) {
                return false;
            }
            break;
        }
        case WorkCondition::Type::TIMER: {
            uint32_t intervalTime = workConditionMap->at(WorkCondition::Type::TIMER)->uintVal;
            time_t lastTime;
            if (s_uid_last_time_map.find(uid_) == s_uid_last_time_map.end()) {
                lastTime = 0;
            } else {
                lastTime = s_uid_last_time_map[uid_];
            }
            double currentdel = difftime(getCurrentTime(), baseTime_) * ONE_SECOND;
            double oppositedel = difftime(getOppositeTime(), lastTime);
            double del = currentdel > oppositedel ? currentdel : oppositedel;
            WS_HILOGD("del time: %{public}lf, intervalTime: %{public}u", del, intervalTime);
            WS_HILOGD("currentdel time: %{public}lf, oppositedel time: %{public}lf", currentdel, oppositedel);
            if (del < intervalTime) {
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

bool WorkStatus::IsNapReady(WorkCondition::Type type)
{
    if (type != WorkCondition::Type::DEEP_IDLE) {
        return true;
    }
    auto conditionSet = workInfo_->GetConditionMap()->at(WorkCondition::Type::DEEP_IDLE);
    auto conditionCurrent = conditionMap_.at(WorkCondition::Type::DEEP_IDLE);
    if (conditionSet->boolVal != conditionCurrent->boolVal) {
        return false;
    }
    return true;
}

bool WorkStatus::SetMinInterval()
{
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    int32_t group = 0;
    if (workInfo_->IsCallBySystemApp()) {
        WS_HILOGD("Is system app, default group is active.");
        return SetMinIntervalByGroup(ACTIVE_GROUP);
    }
    bool res = DelayedSingleton<DataManager>::GetInstance()->FindGroup(bundleName_, userId_, group);
    if (!res) {
        WS_HILOGI("No cache find, bundleName:%{public}s", bundleName_.c_str());
        auto errCode = DeviceUsageStats::BundleActiveClient::GetInstance().QueryAppGroup(group, bundleName_, userId_);
        if (errCode != ERR_OK) {
            WS_HILOGE("Query package group failed. userId = %{public}d, bundleName = %{public}s",
                userId_, bundleName_.c_str());
            group = ACTIVE_GROUP;
            DelayedSingleton<DataManager>::GetInstance()->AddGroup(bundleName_, userId_, group);
        }
    }
#else
    int32_t group = ACTIVE_GROUP;
#endif
    return SetMinIntervalByGroup(group);
}

bool WorkStatus::SetMinIntervalByGroup(int32_t group)
{
    groupChanged_ = true;
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    auto itMap = DeviceUsageStats::DeviceUsageStatsGroupMap::groupIntervalMap_.find(group);
    if (itMap != DeviceUsageStats::DeviceUsageStatsGroupMap::groupIntervalMap_.end()) {
        minInterval_ = DeviceUsageStats::DeviceUsageStatsGroupMap::groupIntervalMap_[group];
    } else {
        WS_HILOGE("Query package group interval failed. group:%{public}d, bundleName:%{public}s",
            group, bundleName_.c_str());
        minInterval_ = -1;
    }
#else
    minInterval_ = MIN_INTERVAL_DEFAULT;
#endif
    WS_HILOGD("Set min interval to %{public}" PRId64 " by group %{public}d", minInterval_, group);
    return true;
}

void WorkStatus::SetMinIntervalByDump(int64_t interval)
{
    WS_HILOGD("Set min interval by dump to %{public}" PRId64 "", interval);
    groupDebugMode = interval == 0 ? false : true;
    minInterval_ = interval == 0 ? minInterval_ : interval;
}

int64_t WorkStatus::GetMinInterval()
{
    return minInterval_;
}

void WorkStatus::UpdateUidLastTimeMap()
{
    std::lock_guard<ffrt::mutex> lock(s_uid_last_time_mutex);
    time_t lastTime = getOppositeTime();
    s_uid_last_time_map[uid_] = lastTime;
}

void WorkStatus::ClearUidLastTimeMap(int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(s_uid_last_time_mutex);
    s_uid_last_time_map.erase(uid);
}

bool WorkStatus::IsRunning()
{
    return currentStatus_ == RUNNING;
}

bool WorkStatus::IsPaused()
{
    return paused_;
}

bool WorkStatus::IsReadyStatus()
{
    return currentStatus_ == CONDITION_READY;
}

bool WorkStatus::IsRemoved()
{
    return currentStatus_ == REMOVED;
}

bool WorkStatus::IsLastWorkTimeout()
{
    return lastTimeout_;
}

bool WorkStatus::IsRepeating()
{
    std::lock_guard<ffrt::mutex> lock(conditionMapMutex_);
    if (conditionMap_.count(WorkCondition::Type::TIMER) <= 0) {
        return false;
    }
    if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
        return true;
    } else {
        return conditionMap_.at(WorkCondition::Type::TIMER)->intVal > 0;
    }
}

WorkStatus::Status WorkStatus::GetStatus()
{
    return currentStatus_;
}

int WorkStatus::GetPriority()
{
    if ((OHOS::system::GetIntParameter("const.debuggable", 0) == 1) &&
        (bundleName_ == "com.huawei.hmos.hiviewx")) {
        return HIGH_PRIORITY;
    }
    return DEFAULT_PRIORITY;
}

void WorkStatus::Dump(string& result)
{
    result.append("{\n");
    result.append(string("\"workId\":") + workId_ + ",\n");
    result.append(string("\"bundleName\":") + bundleName_ + ",\n");
    result.append(string("\"status\":") + to_string(currentStatus_) + ",\n");
    result.append(string("\"paused\":") + (paused_ ? "true" : "false") + ",\n");
    result.append(string("\"priority\":") + to_string(priority_) + ",\n");
    result.append(string("\"conditionMap\":{\n"));
    std::lock_guard<ffrt::mutex> lock(conditionMapMutex_);
    if (conditionMap_.count(WorkCondition::Type::NETWORK) > 0) {
        result.append(string("\"networkType\":") +
            to_string(conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::CHARGER) > 0) {
        result.append(string("\"isCharging\":") +
            (conditionMap_.at(WorkCondition::Type::CHARGER)->boolVal ? "true" : "false") + ",\n");
        result.append(string("\"chargerType\":") +
            to_string(conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::BATTERY_LEVEL) > 0) {
        result.append(string("\"batteryLevel\":") +
            to_string(conditionMap_.at(WorkCondition::Type::BATTERY_LEVEL)->intVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::BATTERY_STATUS) > 0) {
        result.append(string("\"batteryStatus\":") +
            to_string(conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::STORAGE) > 0) {
        result.append(string("\"storageLevel\":") +
            to_string(conditionMap_.at(WorkCondition::Type::STORAGE)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        result.append(string("\"baseTime\":") + to_string(baseTime_) + ",\n");
        if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
            result.append(string("\"isRepeat\": true,\n"));
        } else {
            result.append(string("\"cycleLeft\":") +
                to_string(conditionMap_.at(WorkCondition::Type::TIMER)->intVal) + ",\n");
        }
    }
    if (conditionMap_.count(WorkCondition::Type::DEEP_IDLE) > 0) {
        result.append(string("\"isDeepIdle\":") +
            to_string(conditionMap_.at(WorkCondition::Type::DEEP_IDLE)->boolVal) + ",\n");
    }
    result.append("},\n\"workInfo\":\n");
    workInfo_->Dump(result);
    result.append("}\n");
    result.append("\n");
}

bool WorkStatus::IsUnLockReady()
{
    if (WorkSchedUtils::IsDebugMode()) {
        WS_HILOGI("Debug mode allow to run work");
        return true;
    }
    if (WorkSchedUtils::IsUnlock()) {
        if (WorkSchedUtils::IsBetaVersion()
            && DelayedSingleton<WorkSchedulerService>::GetInstance()->ExemptionBundle(bundleName_)) {
            WS_HILOGI("Beta version and exemption bundle:%{public}s, allow to run work", bundleName_.c_str());
            return true;
        }
        WS_HILOGI("Unlock state not allow to run work");
        return false;
    }
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS