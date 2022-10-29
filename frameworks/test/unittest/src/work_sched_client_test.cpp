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

#include <functional>
#include <gtest/gtest.h>

#include "workscheduler_srv_client.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkSchedClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: WorkSchedClientTest_001
 * @tc.desc: Test StartWork
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_001, TestSize.Level1)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_002
 * @tc.desc: Test StopWork
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_002, TestSize.Level1)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_003
 * @tc.desc: Test StopAndCancelWork
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_003, TestSize.Level1)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_004
 * @tc.desc: Test StopAndClearWorks
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_004, TestSize.Level1)
{
    auto ret = WorkSchedulerSrvClient::GetInstance().StopAndClearWorks();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_005
 * @tc.desc: Test IsLastWorkTimeout
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_005, TestSize.Level1)
{
    int32_t workId = 1;
    bool result;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().IsLastWorkTimeout(workId, result);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_006
 * @tc.desc: Test GetWorkStatus
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_006, TestSize.Level1)
{
    int32_t workId = 1;
    std::shared_ptr<WorkInfo> work;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().GetWorkStatus(workId, work);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_007
 * @tc.desc: Test ObtainAllWorks
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_007, TestSize.Level1)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(workInfos);
    EXPECT_EQ(ret, ERR_OK);
}
}  // namespace WorkScheduler
}  // namespace OHOS