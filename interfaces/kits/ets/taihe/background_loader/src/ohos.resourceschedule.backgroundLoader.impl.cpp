/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

 #include "ohos.resourceschedule.backgroundLoader.impl.h"
#include "workscheduler_srv_client.h"
#include "background_loader_task_info.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "common.h"

void RegisterTask(const ::ohos::resourceschedule::backgroundLoader::TaskInfo& taskInfo)
{
    WS_HILOGI("RegisterTask called");
    std::string abilityName = taskInfo.abilityName;
    BackgroundLoaderTaskInfo info (taskInfo.taskId, abilityName);

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance.RegisterTask(info);
    if (errCode != ERR_OK) {
        WS_HILOGE("RegisterTask fail errCode: %{public}d", FindErrCode(errCode));
        set_business_error(FindErrCode(errCode), FindErrMsg(errCode))
    }
}

void UnregisterTask(const ::ohos::resourceschedule::backgroundLoader::TaskInfo& taskInfo)
{
    WS_HILOGI("UnregisterTask called");
    std::string abilityName = taskInfo.abilityName;
    BackgroundLoaderTaskInfo info (taskInfo.taskId, abilityName);

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance.UnregisterTask(info);
    if (errCode != ERR_OK) {
        WS_HILOGE("UnregisterTask fail errCode: %{public}d", FindErrCode(errCode));
        set_business_error(FindErrCode(errCode), FindErrMsg(errCode))
    }
}

void FinishTask(const ::ohos::resourceschedule::backgroundLoader::TaskInfo& taskInfo)
{
    WS_HILOGI("FinishTask called");
    std::string abilityName = taskInfo.abilityName;
    BackgroundLoaderTaskInfo info (taskInfo.taskId, abilityName);

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance.FinishTask(info);
    if (errCode != ERR_OK) {
        WS_HILOGE("FinishTask fail errCode: %{public}d", FindErrCode(errCode));
        set_business_error(FindErrCode(errCode), FindErrMsg(errCode))
    }
}

::ohos::resourceschedule::backgroundLoader::TaskInfo GetTaskInfoPromise(int32_t taskId)
{
    WS_HILOGI("GetTaskInfoPromise called taskId %{public}d", taskId);
    std::string abilityName = taskInfo.abilityName;
    BackgroundLoaderTaskInfo info;

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance.GetTaskInfo(taskId, info);
    if (errCode != ERR_OK) {
        WS_HILOGE("GetTaskInfoPromise fail errCode: %{public}d", FindErrCode(errCode));
        set_business_error(FindErrCode(errCode), FindErrMsg(errCode))
    }
    ::ohos::resourceschedule::backgroundLoader::TaskInfo taskInfo(info.taskId, info.abilityName);
    return info;
}