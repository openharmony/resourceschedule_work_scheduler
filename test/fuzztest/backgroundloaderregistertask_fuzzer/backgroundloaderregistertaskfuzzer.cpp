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

#include <cstddef>
#include <cstdint>
#include <securec.h>
#include "message_parcel.h"
#include "background_loader_task_info.h"
#include "i_work_sched_service.h"

namespace OHOS::WorkScheduler {
bool DoBackgroundLoaderRegisterTaskFuzz(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    MessageParcel parcel;
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);

    int32_t taskId = parcel.ReadInt32();
    std::string abilityName = parcel.ReadString();

    BackgroundLoaderTaskInfo taskInfo(taskId, abilityName);

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    uint32_t code = static_cast<uint32_t>(IWorkSchedService::Code::REGISTER_TASK);
    sptr<IRemoteObject> remote = nullptr;

    parcel.RewindRead(0);
    parcel.WriteInt32(taskId);
    parcel.WriteString(abilityName);

    MessageParcel dataParcel;
    dataParcel.WriteInterfaceToken(IWorkSchedService::GetDescriptor());
    dataParcel.WriteBuffer(data, size);
    dataParcel.RewindRead(0);
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoBackgroundLoaderRegisterTaskFuzz(data, size);
    return 0;
}