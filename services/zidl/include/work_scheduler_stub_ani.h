/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#ifndef OHOS_ZIDL_WORK_SCHED_STUB_ANI_H
#define OHOS_ZIDL_WORK_SCHED_STUB_ANI_H
 
#include <memory>
#include "ani_work_scheduler_extension.h"
#include "work_scheduler_stub.h"
#include "work_sched_hilog.h"
 
namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerStubAni : public WorkSchedulerStub {
public:
    explicit WorkSchedulerStubAni(const std::shared_ptr<AniWorkSchedulerExtension>& extension)
        : extension_(extension) {}
 
    ~WorkSchedulerStubAni() override {}
 
    /**
     * @brief The OnWorkStart callback.
     *
     * @param workInfo The info of work.
     */
    ErrCode OnWorkStart(const WorkInfo& workInfo) override;
 
    /**
     * @brief The OnWorkStop callback.
     *
     * @param workInfo The info of work.
     */
    ErrCode OnWorkStop(const WorkInfo& workInfo) override;
private:
    std::weak_ptr<AniWorkSchedulerExtension> extension_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // OHOS_ZIDL_WORK_SCHED_STUB_ANI_H