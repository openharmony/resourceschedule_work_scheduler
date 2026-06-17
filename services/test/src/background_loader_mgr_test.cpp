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

#include <gtest/gtest.h>
#include "background_loader_mgr.h"
#include "work_sched_errors.h"

using namespace testing::ext;
using namespace OHOS::WorkScheduler;
using namespace OHOS;
class BackgroundLoaderMgrTest : public testing::Test {
public:
    void SetUp() override
    {
        BackgroundLoaderMgr::GetInstance().Init();
    };
    void TearDown() override {};
};

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_001
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_NotReady_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_Duplicate_001
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask duplicate task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_Duplicate_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_NotExist_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask when task not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_NotExist_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 999
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_NotReady_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
    BackgroundLoaderMgr::GetInstance().Init();
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_NotExist_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask when task not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_NotExist_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 999
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_NotReady_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
    BackgroundLoaderMgr::GetInstance().Init();
}

/**
 * @tc.name: BackgroundLoaderMgr_GetTaskInfo_001
 * @tc.desc: Test BackgroundLoaderMgr GetTaskInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfo_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(taskInfo.GetTaskId(), 1);
    EXPECT_EQ(taskInfo.GetAbilityName(), "TestAbility");
}

/**
 * @tc.name: BackgroundLoaderMgr_GetTaskInfo_NotExist_001
 * @tc.desc: Test BackgroundLoaderMgr GetTaskInfo when task not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfo_NotExist_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo;
    ErrCode ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(999, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_GetTaskInfo_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr GetTaskInfo when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfo_NotReady_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo;
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
    BackgroundLoaderMgr::GetInstance().Init();
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterUnregisterGetTask_001
 * @tc.desc: Test BackgroundLoaderMgr register then unregister then get task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterUnregisterGetTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 10
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(10, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, ERR_OK);

    ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(10, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_MultipleTasks_001
 * @tc.desc: Test BackgroundLoaderMgr register multiple tasks.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, MultipleTasks_001, TestSize.Level1)
{
    for (int32_t i = 1; i <= 5; i++) {
        TaskInfo info = {
            .bundleName_ = "com.test.bundle",
            .abilityName_ = "Ability_" + std::to_string(i),
            .appIndex_ = 0,
            .taskId_ = i
        };
        ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
        EXPECT_EQ(ret, ERR_OK);
    }

    for (int32_t i = 1; i <= 5; i++) {
        BackgroundLoaderTaskInfo taskInfo;
        ErrCode ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(i, "com.test.bundle", 0, taskInfo);
        EXPECT_EQ(ret, ERR_OK);
        EXPECT_EQ(taskInfo.GetTaskId(), i);
        EXPECT_EQ(taskInfo.GetAbilityName(), "Ability_" + std::to_string(i));
    }
}

/**
 * @tc.name: BackgroundLoaderMgr_DifferentAppIndex_001
 * @tc.desc: Test BackgroundLoaderMgr with same taskId but different appIndex.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, DifferentAppIndex_001, TestSize.Level1)
{
    TaskInfo info1 = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    TaskInfo info2 = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 1,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info1);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info2);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo1;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 0, taskInfo1);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo2;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 1, taskInfo2);
    EXPECT_EQ(ret, ERR_OK);
}