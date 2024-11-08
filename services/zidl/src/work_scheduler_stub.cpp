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

#include "work_scheduler_stub.h"
#include "work_scheduler_stub_ipc_interface_code.h"

namespace OHOS {
namespace WorkScheduler {
__attribute__((no_sanitize("cfi"))) int32_t WorkSchedulerStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        WS_HILOGE("failed, descriptor is not matched!");
        return ERR_INVALID_STATE;
    }
    switch (code) {
        case static_cast<uint32_t>(WorkSchedulerStubInterfaceCode::COMMAND_ON_WORK_START): {
            sptr<WorkInfo> workInfo = data.ReadStrongParcelable<WorkInfo>();
            if (workInfo == nullptr) {
                WS_HILOGE("workInfo is nullptr");
                return ERR_TRANSACTION_FAILED;
            }
            OnWorkStart(*workInfo);
            return ERR_NONE;
        }
        case static_cast<uint32_t>(WorkSchedulerStubInterfaceCode::COMMAND_ON_WORK_STOP): {
            sptr<WorkInfo> workInfo = data.ReadStrongParcelable<WorkInfo>();
            if (workInfo == nullptr) {
                WS_HILOGE("workInfo is nullptr");
                return ERR_TRANSACTION_FAILED;
            }
            OnWorkStop(*workInfo);
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_OK;
}
} // namespace WorkScheduler
} // namespace OHOS
