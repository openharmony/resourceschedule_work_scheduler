/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "background_loader_mgr.h"
#include "work_sched_errors.h"
#include "work_sched_hilog.h"
#include "ffrt.h"

#include <ipc_skeleton.h>
#include <system_ability.h>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include "message_parcel.h"
#include "message_option.h"
#include "want.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr std::string_view TIMEOUT_MESSAGE = "timeOut";
constexpr std::string_view TIMEOUT_TASK_NAME = "BackgroundLoaderTimeout";
}
using namespace OHOS::ResrouceSchedule;
BackgroundLoaderMgr& BackgroundLoaderMgr::GetInstance()
{
    static auto instance = new BackgroundLoaderMgr();
    return *instance;
}

void BackgroundLoaderMgr::init()
{
    WS_HILOGI("BackgroundLoaderMgr init");
    isReady_.store(true);
}

std::string BackgroundLoaderMgr::GenerateTaskKey(const std::string& bundlelName, int32_t appIndex, int32_t taskId)
{
    return bundleName + "_" + std::to_string(taskId) + "_" + std::to_string(appIndex);
}

ErrCode BackgroundLoaderMgr::RegisterTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId, taskInfo.bundleName.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<std::mutex> lock(taskLock);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_, taskInfo.taskId_);
    taskMap_[key] = taskInfo;

    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::UnregisterTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId, taskInfo.bundleName.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<std::mutex> lock(taskLock);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_, taskInfo.taskId_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end()) {
        WS_HILOGE("UnregisterTask failed : task not found");
        return E_WORK_NOT_EXIST_FAILED;
    }

    taskMap_.erase(it);
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::FinishTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId, taskInfo.bundleName.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<std::mutex> lock(taskLock);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_, taskInfo.taskId_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end()) {
        WS_HILOGE("FinishTask failed : task not found");
        return E_WORK_NOT_EXIST_FAILED;
    }

    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::GetTaskInfo(int32_t taskId, const std::string& bundleName, int32_t appIndex,
    BackgroundLoaderTaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId, taskInfo.bundleName.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<std::mutex> lock(taskLock);
    std::string key = GenerateTaskKey(bundleName, taskInfo.appIndex, taskId_);
    auto it = taskMap_.find(key);
    if (it != taskMap_.end()) {
        TaskInfo info = it->secound;
        BackgroundLoaderTaskInfo newInfo(info.taskId_, inf.abilityName);
        return ERR_OK;
    }

    WS_HILOGE("GetTaskInfo failed : task not found");
    return E_WORK_NOT_EXIST_FAILED;
}

}  // namespace WorkScheduler
}  // namespace OHOS