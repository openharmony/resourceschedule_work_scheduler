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
#ifdef POWERMGR_THERMAL_MANAGER_ENABLE
#include "policy/thermal_policy.h"

#include "thermal_mgr_client.h"
#include "work_sched_hilog.h"

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace WorkScheduler {
const int32_t COUNT_THERMAL_CRUCIAL = 0;
const int32_t COUNT_THERMAL_LOW = 1;
const int32_t COUNT_THERMAL_MIDDLE = 2;
const int32_t COUNT_THERMAL_NORMAL = 3;

ThermalPolicy::ThermalPolicy(shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

ThermalPolicy::~ThermalPolicy()
{
}

int32_t ThermalPolicy::GetThermalLevel()
{
    if (workPolicyManager_ != nullptr) {
        int32_t dumpThermalLevel = workPolicyManager_->GetDumpSetThermalLevel();
        if (dumpThermalLevel >= 0) {
            WS_HILOGI("dump set thermalLevel: %{public}d", dumpThermalLevel);
            return dumpThermalLevel;
        }
    }
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel thermalLevel = thermalMgrClient.GetThermalLevel();
    return static_cast<int32_t>(thermalLevel);
}

int32_t ThermalPolicy::GetCurThermalLevelMaxRunning(int32_t thermalLevel)
{
    int32_t res;
#ifndef PC_PLATFORM
    if (thermalLevel >= static_cast<int32_t>(ThermalLevel::WARNING)) {
        res = COUNT_THERMAL_CRUCIAL;
    } else if (thermalLevel < static_cast<int32_t>(ThermalLevel::WARNING) &&
        thermalLevel >= static_cast<int32_t>(ThermalLevel::OVERHEATED)) {
        res = COUNT_THERMAL_LOW;
    } else if (thermalLevel < static_cast<int32_t>(ThermalLevel::OVERHEATED) &&
        thermalLevel >= static_cast<int32_t>(ThermalLevel::HOT)) {
        res = COUNT_THERMAL_MIDDLE;
    } else {
        res = COUNT_THERMAL_NORMAL;
    }
#else
    if (thermalLevel >= static_cast<int32_t>(ThermalLevel::WARM)) {
        res = COUNT_THERMAL_CRUCIAL;
    } else if (thermalLevel < static_cast<int32_t>(ThermalLevel::WARM) &&
        thermalLevel >= static_cast<int32_t>(ThermalLevel::NORMAL)) {
        res = COUNT_THERMAL_LOW;
    } else {
        res = COUNT_THERMAL_NORMAL;
    }
#endif
    return res;
}

int32_t ThermalPolicy::GetPolicyMaxRunning(WorkSchedSystemPolicy& systemPolicy)
{
    int32_t thermalLevel = GetThermalLevel();
    int32_t res = GetCurThermalLevelMaxRunning(thermalLevel);
    systemPolicy.thermalLevel = thermalLevel;
    systemPolicy.SetPolicyName("THERMAL_POLICY", res);
    WS_HILOGD("ThermalLevel:%{public}d, PolicyRes:%{public}d", thermalLevel, res);
    return res;
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_THERMAL_MANAGER_ENABLE