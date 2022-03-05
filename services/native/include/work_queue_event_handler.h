/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WORK_SCHED_SERVICES_WORK_QUEUE_EVENT_HANDLER_H
#define WORK_SCHED_SERVICES_WORK_QUEUE_EVENT_HANDLER_H

#include <event_handler.h>
#include <refbase.h>

#include "work_queue_manager.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueEventHandler : public AppExecFwk::EventHandler {
public:
    enum {
        TIMER_TICK = 0,
    };
    WorkQueueEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
        std::shared_ptr<WorkQueueManager> manager);
    ~WorkQueueEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::shared_ptr<WorkQueueManager> manager_;
};
} // namespace WorkScheduler
} // namespace OHOS

#endif // WORK_SCHED_SERVICES_WORK_QUEUE_EVENT_HANDLER_H