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

#ifndef WORKSCHED_EXTENSION_CONTEXT_H
#define WORKSCHED_EXTENSION_CONTEXT_H

#include "extension_context.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerExtensionContext : public AbilityRuntime::ExtensionContext {
public:
    WorkSchedulerExtensionContext();

    ~WorkSchedulerExtensionContext() override;
    ErrCode StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1) const;
    ErrCode StopServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1) const;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // WORKSCHED_EXTENSION_CONTEXT_H