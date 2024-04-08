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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_THERMAL_POLICY_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_THERMAL_POLICY_H

#ifdef POWERMGR_THERMAL_MANAGER_ENABLE
#include "ipolicy_filter.h"
#include "work_policy_manager.h"

namespace OHOS {
namespace WorkScheduler {
class ThermalPolicy : public IPolicyFilter {
public:
    explicit ThermalPolicy(std::shared_ptr<WorkPolicyManager> workPolicyManager);
    ~ThermalPolicy() override;
    /**
     * @brief Get policy max running.
     *
     * @return Res.
     */
    int32_t GetPolicyMaxRunning() override;
    
    /**
     * @brief Get policyName.
     *
     * @return PolicyName.
     */
    std::string GetPolicyName() override;
private:
    std::shared_ptr<WorkPolicyManager> workPolicyManager_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_THERMAL_MANAGER_ENABLE
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_THERMAL_POLICY_H