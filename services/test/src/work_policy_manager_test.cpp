/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <functional>
#include <gtest/gtest.h>

#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_status.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
#include "watchdog.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
};

std::shared_ptr<WorkPolicyManager> WorkPolicyManagerTest::workPolicyManager_ = nullptr;

void WorkPolicyManagerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
}

/**
 * @tc.name: RealStartWork_001
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I8OLHT
 */
HWTEST_F(WorkPolicyManagerTest, RealStartWork_001, TestSize.Level1)
{
    WorkInfo workinfo;
    int32_t uid;
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->RealStartWork(topWork);
    EXPECT_FALSE(topWork->IsRunning());
}

/**
 * @tc.name: CheckWorkToRun_001
 * @tc.desc: Test WorkPolicyManagerTest CheckWorkToRun.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, CheckWorkToRun_001, TestSize.Level1)
{
    workPolicyManager_->conditionReadyQueue_->ClearAll();
    workPolicyManager_->CheckWorkToRun();
    EXPECT_TRUE(workPolicyManager_->conditionReadyQueue_->GetSize() == 0);
}

/**
 * @tc.name: AddWork_001
 * @tc.desc: Test WorkPolicyManagerTest AddWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AddWork_002
 * @tc.desc: Test WorkPolicyManagerTest AddWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);

    WorkInfo workinfo1;
    workinfo1.SetWorkId(10000);
    workinfo1.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo1.RequestBatteryLevel(80);
    std::shared_ptr<WorkStatus> workStatus1 = std::make_shared<WorkStatus>(workinfo1, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus1, uid);

    EXPECT_EQ(ret, E_ADD_REPEAT_WORK_ERR);
}

/**
 * @tc.name: AddWork_003
 * @tc.desc: Test WorkPolicyManagerTest AddWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    int32_t uid = 10000;
    for (int32_t i = 0; i < 10; i++) {
        WorkInfo workinfo;
        workinfo.SetWorkId(10000 + i);
        workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
        workinfo.RequestBatteryLevel(80);
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
        workPolicyManager_->AddWork(workStatus, uid);
    }

    WorkInfo workinfo1;
    workinfo1.SetWorkId(10010);
    workinfo1.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo1.RequestBatteryLevel(80);
    std::shared_ptr<WorkStatus> workStatus1 = std::make_shared<WorkStatus>(workinfo1, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus1, uid);

    EXPECT_EQ(ret, E_WORK_EXCEED_UPPER_LIMIT);
}

/**
 * @tc.name: RemoveWork_001
 * @tc.desc: Test WorkPolicyManagerTest RemoveWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    bool ret = workPolicyManager_->RemoveWork(workStatus, uid);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: RemoveWork_002
 * @tc.desc: Test WorkPolicyManagerTest RemoveWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->RemoveWork(workStatus, uid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: RemoveWork_003
 * @tc.desc: Test WorkPolicyManagerTest RemoveWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid1 = 10001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid1);
    workPolicyManager_->AddWork(workStatus, uid1);

    int32_t uid2 = 10002;
    bool ret = workPolicyManager_->RemoveWork(workStatus, uid2);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: PauseRunningWorks_001
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_001, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    int32_t uid = 10000;
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: PauseRunningWorks_002
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_002, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: PauseRunningWorks_003
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_003, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->paused_ = true;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PauseRunningWorks_004
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_004, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->paused_ = false;
    workStatus->workWatchDogTime_ = 120000;
    workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs();
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::shared_ptr<Watchdog> watchdog_ =
        std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
    workPolicyManager_->watchdog_ = watchdog_;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ResumePausedWorks_001
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_001, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    int32_t uid = 10000;
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: ResumePausedWorks_002
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_002, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: ResumePausedWorks_003
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_003, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->paused_ = false;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ResumePausedWorks_004
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_004, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->paused_ = true;
    workStatus->workWatchDogTime_ = 120000;
    workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs();
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::shared_ptr<Watchdog> watchdog_ =
        std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
    workPolicyManager_->watchdog_ = watchdog_;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetConditionString_001
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_001, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetConditionString_002
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_002, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: GetConditionString_003
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_003, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: GetConditionString_004
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_004, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: GetConditionString_005
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_005, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryLevel(29);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: GetConditionString_006
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_006, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: GetConditionString_007
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_007, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestRepeatCycle(1200000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: GetConditionString_008
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_008, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: FindWorkStatus_001
 * @tc.desc: Test WorkPolicyManagerTest FindWorkStatus.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWorkStatus_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);
    std::shared_ptr<WorkStatus> ret = workPolicyManager_->FindWorkStatus(workinfo, uid);
    EXPECT_FALSE(ret == nullptr);
}

/**
 * @tc.name: FindWorkStatus_002
 * @tc.desc: Test WorkPolicyManagerTest FindWorkStatus.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWorkStatus_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);
    
    int32_t uid1 = 10001;
    std::shared_ptr<WorkStatus> ret = workPolicyManager_->FindWorkStatus(workinfo, uid1);
    EXPECT_TRUE(ret == nullptr);
}

/**
 * @tc.name: StopWork_001
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    bool ret = workPolicyManager_->StopWork(workStatus, uid, false, false);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: StopWork_002
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestRepeatCycle(1200000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    bool ret = workPolicyManager_->StopWork(workStatus, uid, true, false);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: StopWork_003
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestRepeatCycle(1200000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    bool ret = workPolicyManager_->StopWork(workStatus, uid, true, false);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: StopWork_004
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_004, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    bool ret = workPolicyManager_->StopWork(workStatus, uid, false, true);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: StopAndClearWorks_001
 * @tc.desc: Test WorkPolicyManagerTest StopAndClearWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopAndClearWorks_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->StopAndClearWorks(uid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsLastWorkTimeout_001
 * @tc.desc: Test WorkPolicyManagerTest IsLastWorkTimeout.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, IsLastWorkTimeout_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->lastTimeout_ = true;
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = false;
    ret = workPolicyManager_->IsLastWorkTimeout(workId, uid, ret);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionReady_001
 * @tc.desc: Test WorkPolicyManagerTest OnConditionReady.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, OnConditionReady_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<WorkStatus>> readyWorkVector = {};
    std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> readyWork =
        std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(readyWorkVector);
    workPolicyManager_->OnConditionReady(readyWork);
    EXPECT_TRUE(readyWork != nullptr);
}

/**
 * @tc.name: OnConditionReady_002
 * @tc.desc: Test WorkPolicyManagerTest OnConditionReady.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, OnConditionReady_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<WorkStatus>> readyWorkVector;
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->lastTimeout_ = true;
    readyWorkVector.emplace_back(workStatus);
    std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> readyWork =
        std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(readyWorkVector);
    workPolicyManager_->OnConditionReady(readyWork);
    EXPECT_FALSE(readyWork == nullptr);
}

/**
 * @tc.name: AddToReadyQueue_001
 * @tc.desc: Test WorkPolicyManagerTest AddToReadyQueue.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddToReadyQueue_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<WorkStatus>> readyWorkVector;
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->lastTimeout_ = true;
    readyWorkVector.emplace_back(workStatus);
    std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> readyWork =
        std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(readyWorkVector);
    workPolicyManager_->AddToReadyQueue(readyWork);
    EXPECT_FALSE(readyWork == nullptr);
}

/**
 * @tc.name: GetMaxRunningCount_001
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_001, TestSize.Level1)
{
    workPolicyManager_->dumpSetMaxRunningCount_ = 2;
    std::string policyName;
    int32_t ret = workPolicyManager_->GetMaxRunningCount(policyName);
    EXPECT_TRUE(ret == workPolicyManager_->dumpSetMaxRunningCount_);
}

/**
 * @tc.name: GetMaxRunningCount_002
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_002, TestSize.Level1)
{
    workPolicyManager_->dumpSetMaxRunningCount_ = 0;
    std::string policyName;
    int32_t ret = workPolicyManager_->GetMaxRunningCount(policyName);
    EXPECT_TRUE(ret >= workPolicyManager_->dumpSetMaxRunningCount_);
}

/**
 * @tc.name: GetRunningCount_001
 * @tc.desc: Test WorkPolicyManagerTest GetRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetRunningCount_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    int32_t ret = workPolicyManager_->GetRunningCount();
    EXPECT_TRUE(ret == 1);
}

/**
 * @tc.name: FindWork_001
 * @tc.desc: Test WorkPolicyManagerTest FindWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->FindWork(uid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: FindWork_002
 * @tc.desc: Test WorkPolicyManagerTest FindWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.bundleName_ = "com.test.demo";
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->userId_ = 100;
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->FindWork(workStatus->userId_, workStatus->bundleName_);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: FindWork_003
 * @tc.desc: Test WorkPolicyManagerTest FindWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.bundleName_ = "com.test.demo";
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->userId_ = 100;

    bool ret = workPolicyManager_->FindWork(workStatus->userId_, workStatus->bundleName_);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetDeepIdleWorks_001
 * @tc.desc: Test WorkPolicyManagerTest GetDeepIdleWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetDeepIdleWorks_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.bundleName_ = "com.test.demo";
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->userId_ = 100;
    workPolicyManager_->AddWork(workStatus, uid);

    std::list<std::shared_ptr<WorkStatus>> ret = workPolicyManager_->GetDeepIdleWorks();
    EXPECT_TRUE(ret.size() > 0);
}
}
}