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

#include "policy/app_data_clear_listener.h"

#include "common_event_support.h"
#include "common_event_manager.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"
#include "work_scheduler_service.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
static const std::string UID_PARAM = "uid";

AppDataClearSubscriber::AppDataClearSubscriber(const CommonEventSubscribeInfo &subscribeInfo,
    AppDataClearListener &listener) : CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void AppDataClearSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    const string action = data.GetWant().GetAction();
    string bundle = data.GetWant().GetBundle();
    int32_t uid = data.GetWant().GetIntParam(UID_PARAM, -1);
    WS_HILOGI("OnReceiveEvent get action: %{public}s, bundleName: %{public}s , uid: %{public}d",
        action.c_str(), bundle.c_str(), uid);
    auto detectorVal = make_shared<DetectorValue>(uid, 0, 0, bundle);
    if (action == CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED) {
        listener_.OnPolicyChanged(PolicyType::APP_DATA_CLEAR, detectorVal);
    } else if (action == CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        listener_.OnPolicyChanged(PolicyType::APP_REMOVED, detectorVal);
    } else if (action == CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        listener_.OnPolicyChanged(PolicyType::APP_CHANGED, detectorVal);
    } else if (action == CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        listener_.OnPolicyChanged(PolicyType::APP_ADDED, detectorVal);
    } else if (action == CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        listener_.OnPolicyChanged(PolicyType::USER_SWITCHED, detectorVal);
    }
}

shared_ptr<CommonEventSubscriber> CreateAppDataClearSubscriber(AppDataClearListener &listener)
{
    MatchingSkills skill = MatchingSkills();
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    CommonEventSubscribeInfo info(skill);
    return make_shared<AppDataClearSubscriber>(info, listener);
}

AppDataClearListener::AppDataClearListener(std::shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

AppDataClearListener::~AppDataClearListener()
{
    this->Stop();
}

bool AppDataClearListener::Start()
{
    this->commonEventSubscriber = CreateAppDataClearSubscriber(*this);
    return CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool AppDataClearListener::Stop()
{
    if (this->commonEventSubscriber != nullptr) {
        bool result = CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void AppDataClearListener::OnPolicyChanged(PolicyType policyType, shared_ptr<DetectorValue> detectorVal)
{
    auto handler = DelayedSingleton<WorkSchedulerService>::GetInstance()->GetHandler();
    if (handler) {
        handler->PostTask([weak = weak_from_this(), policyType, detectorVal]() {
            auto strong = weak.lock();
            if (!strong) {
                WS_HILOGE("AppDataClearListener::OnPolicyChanged strong is null");
                return;
            }
            strong->workPolicyManager_->OnPolicyChanged(policyType, detectorVal);
        });
    }
}
} // namespace WorkScheduler
} // namespace OHOS