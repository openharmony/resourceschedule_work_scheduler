/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "work_status.h"
#include "work_condition.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkStatusTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkStatus> workStatus_;
    static std::shared_ptr<WorkInfo> workInfo_;
};

std::shared_ptr<WorkStatus> WorkStatusTest::workStatus_ = nullptr;
std::shared_ptr<WorkInfo> WorkStatusTest::workInfo_ = nullptr;

void WorkStatusTest::SetUpTestCase()
{
    workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->SetWorkId(-1);
    workInfo_->SetElement("com.huawei.ohos.bundlename", "testAbility");
    workInfo_->RequestPersisted(false);
    time_t baseTime;
    (void)time(&baseTime);
    workInfo_->RequestBaseTime(baseTime);
    workStatus_ = std::make_shared<WorkStatus>(*(workInfo_.get()), -1);
}

/**
 * @tc.name: makeWorkId_001
 * @tc.desc: Test WorkStatus MakeWorkId.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, makeWorkId_001, TestSize.Level1)
{
    std::string result = workStatus_->MakeWorkId(1, 1);
    EXPECT_EQ(result, "u1_1");
}

/**
 * @tc.name: isSameUser_001
 * @tc.desc: Test WorkStatus IsSameUser.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isSameUser_001, TestSize.Level1)
{
    workStatus_->uid_ = -1;
    bool result = workStatus_->IsSameUser();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: isSameUser_002
 * @tc.desc: Test WorkStatus IsSameUser.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isSameUser_002, TestSize.Level1)
{
    workStatus_->uid_ = 1;
    bool result = workStatus_->IsSameUser();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isUriKeySwitchOn_001
 * @tc.desc: Test WorkStatus IsUriKeySwitchOn.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isUriKeySwitchOn_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->RequestPersisted(false);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsUriKeySwitchOn();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: isUriKeySwitchOn_002
 * @tc.desc: Test WorkStatus IsUriKeySwitchOn.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isUriKeySwitchOn_002, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->RequestPersisted(true);
    workInfo_->workId_ = 1;
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsUriKeySwitchOn();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isUriKeySwitchOn_003
 * @tc.desc: Test WorkStatus IsUriKeySwitchOn.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isUriKeySwitchOn_003, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = 1;
    workInfo_->RequestPersisted(true);
    workInfo_->uriKey_ = "key";
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsUriKeySwitchOn();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_001
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_002
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_002, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workInfo_->RequestNetworkType(WorkCondition::Network::NETWORK_UNKNOWN);
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_003
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_003, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_BLUETOOTH);
    std::shared_ptr<Condition> networkCondition = std::make_shared<Condition>();
    networkCondition->enumVal = WorkCondition::Network::NETWORK_TYPE_ETHERNET;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::NETWORK, networkCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_004
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_004, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    std::shared_ptr<Condition> batteryCondition = std::make_shared<Condition>();
    batteryCondition->enumVal = WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::NETWORK, batteryCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_005
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_005, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryLevel(60);
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 70;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_006
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_006, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryLevel(60);
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 60;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_007
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_007, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_LOW;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_008
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_008, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    time_t baseTime;
    (void)time(&baseTime);
    workInfo_->RequestBaseTime(baseTime);
    std::shared_ptr<Condition> timerCondition = std::make_shared<Condition>();
    time_t baseTimeTemp;
    (void)time(&baseTimeTemp);
    timerCondition->timeVal = baseTimeTemp;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, timerCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_009
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_009, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0010
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0010, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0011
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0011, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0012
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0012, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0013
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0013, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(false, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0014
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0014, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_AC;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}
}
}