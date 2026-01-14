/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_SCREEN_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_SCREEN_LISTENER_H

#include <memory>

#include "common_event_subscriber.h"
#include "icondition_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueManager;
class ScreenListener : public IConditionListener,
                       public std::enable_shared_from_this<ScreenListener> {
public:
    struct SaTimerInfo {
        /* time_ is setting the idle time delay duration */
        int32_t time_;
        int32_t uid_;
        uint64_t timerId_;
        SaTimerInfo() : time_(0), uid_(0), timerId_(0) {}
        SaTimerInfo(int32_t time, int32_t uid) : time_(time), uid_(uid), timerId_(0) {}
    };
    explicit ScreenListener(std::shared_ptr<WorkQueueManager> workQueueManager,
        std::shared_ptr<WorkSchedulerService> service);
    ~ScreenListener() override;

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
    /**
     * @brief Start timer.
     */
    void StartTimer();
    /**
     * @brief Stop timer.
     */
    void StopTimer();
public:
    std::shared_ptr<WorkSchedulerService> service_;
private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber = nullptr;
    std::map<int32_t, SaTimerInfo> saIdTimeInfoMap_ {};
};

class ScreenEventSubscriber : public EventFwk::CommonEventSubscriber,
                              public std::enable_shared_from_this<ScreenEventSubscriber> {
public:
    ScreenEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo, ScreenListener &listener);
    ~ScreenEventSubscriber() override = default;
    /**
     * @brief The OnReceiveEvent callback.
     *
     * @param data The data.
     */
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
private:
    ScreenListener &listener_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_SCREEN_LISTENER_H