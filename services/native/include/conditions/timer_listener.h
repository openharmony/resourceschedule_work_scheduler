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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_TIMER_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_TIMER_LISTENER_H

#include <memory>
#include <string>

#include "icondition_listener.h"
#include "work_queue_event_handler.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"

namespace OHOS {
namespace WorkScheduler {
class TimerListener : public IConditionListener {
public:
    explicit TimerListener(std::shared_ptr<WorkQueueManager> workQueueManager,
        const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~TimerListener() override = default;
    /**
     * @brief The OnConditionChanged callback.
     *
     * @param conditionType The condition type.
     * @param conditionVal The condition val.
     */
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override;
    /**
     * @brief Start.
     *
     * @return True if success,else false.
     */
    bool Start() override;
    /**
     * @brief Stop.
     *
     * @return True if success,else false.
     */
    bool Stop() override;
private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<WorkQueueEventHandler> handler_;
    const std::string TIMER_LISTENER = "TimerListener";
    uint64_t timerId_ = 0;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_TIMER_LISTENER_H