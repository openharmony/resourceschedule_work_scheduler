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
 
#include "work_sched_config.h"
#include "work_sched_hilog.h"
 
using namespace testing::ext;
 
namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string APP1 = "com.droi.tong";
const std::string APP2 = "com.droi.iapps";
const std::string APP3 = "com.easy.abroadHarmony.temp";
const std::string APP4 = "com.easy.hmos.abroad";
const std::string APP5 = "test";
}
 
class WorkSchedConfigTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
 
/**
 * @tc.name: InitActiveGroupWhitelist_001
 * @tc.desc: Test EventPublisher PublishEvent.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(WorkSchedConfigTest, InitActiveGroupWhitelist_001, TestSize.Level1)
{
    std::string configData = "";
    DelayedSingleton<WorkSchedulerConfig>::GetInstance()->InitActiveGroupWhitelist(configData);
    EXPECT_FALSE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist(APP1));
    configData = "{\"active_group_whitelist\":["
                        "\"com.droi.tong\","
                        "\"com.droi.iapps\","
                        "\"com.easy.abroadHarmony.temp\","
                        "\"com.easy.hmos.abroad\""
                    "]"
                "}";
    DelayedSingleton<WorkSchedulerConfig>::GetInstance()->InitActiveGroupWhitelist(configData);
    EXPECT_TRUE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist(APP1));
    EXPECT_TRUE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist(APP2));
    EXPECT_TRUE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist(APP3));
    EXPECT_TRUE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist(APP4));
    EXPECT_FALSE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist(APP5));
}
}
}