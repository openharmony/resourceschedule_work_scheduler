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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <string>

#include <iremote_object.h>
#include <nocopyable.h>
#include <ipc_skeleton.h>

#include "work_sched_errors.h"
#include "background_loader_task_info.h"
#include "nlohmann/json.hpp"
#include "single_instance.h"

#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
#include "ability_connect_callback_stub.h"
#include "want.h"
#include "ability_manager_client.h"
#endif

namespace OHOS {
namespace WorkScheduler {

struct TaskInfo {
    std::string bundleName_;
    std::string abilityName_;
    int32_t appIndex_;
    int32_t taskId_;
};

class BackgroundLoaderMgr {
DECLARE_SINGLE_INSTANCE(BackgroundLoaderMgr);
public:
    void Init();
    ErrCode RegisterTask(const TaskInfo& taskInfo);
    ErrCode UnregisterTask(const TaskInfo& taskInfo);
    ErrCode FinishTask(const TaskInfo& taskInfo);
    ErrCode GetTaskInfo(int32_t taskId, const std::string& bundleName, int32_t appIndex,
        BackgroundLoaderTaskInfo& taskInfo);
    int32_t GetTaskId(const std::string& bundleName, const std::string& abilityName, int32_t appIndex);
private:
    std::string GenerateTaskKey(const std::string& bundleName, int32_t appIndex, int32_t taskId);

    std::atomic<bool> isReady_ {false};
    std::mutex taskLock_;
    std::unordered_map<std::string, TaskInfo> taskMap_;
    std::mutex abilityMapLock_;
};

}  // namespace WorkScheduler
}  // namespace OHOS


#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H