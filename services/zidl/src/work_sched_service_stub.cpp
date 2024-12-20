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
#include "work_sched_service_stub.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "work_sched_common.h"
#include "work_sched_errors.h"

#ifdef HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#define XCOLLIE_TIMEOUT_SECONDS 10
#endif

namespace OHOS {
namespace WorkScheduler {
int32_t WorkSchedServiceStub::HandleObtainAllWorksRequest(MessageParcel &data, MessageParcel &reply)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    int32_t ret = ObtainAllWorksStub(data, workInfos);
    uint32_t worksize = workInfos.size();
    reply.WriteInt32(ret);
    reply.WriteInt32(worksize);
    for (auto workInfo : workInfos) {
        reply.WriteParcelable(&*workInfo);
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::HandleGetWorkStatusRequest(MessageParcel &data, MessageParcel &reply)
{
    WS_HILOGI("call GetWorkStatus");
    std::shared_ptr<WorkInfo> workInfo;
    int32_t ret = GetWorkStatusStub(data, workInfo);
    reply.WriteInt32(ret);
    reply.WriteParcelable(&*workInfo);
    return ERR_OK;
}

int32_t WorkSchedServiceStub::HandleGetAllRunningWorksRequest(MessageParcel &reply)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    int32_t ret = GetAllRunningWorksStub(workInfos);
    uint32_t worksize = workInfos.size();
    reply.WriteInt32(ret);
    reply.WriteInt32(worksize);
    for (auto workInfo : workInfos) {
        reply.WriteParcelable(&*workInfo);
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::HandleIsLastWorkTimeOutRequest(MessageParcel &data, MessageParcel &reply)
{
    bool isLastWorkTimeout;
    int32_t ret = IsLastWorkTimeoutStub(data, isLastWorkTimeout);
    if (!reply.WriteInt32(ret)) {
        WS_HILOGE("Handle IsLastWorkTimeOut request failed, write result error");
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!reply.WriteBool(isLastWorkTimeout)) {
        WS_HILOGE("Handle IsLastWorkTimeOut request failed, write result error");
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}

int32_t WorkSchedServiceStub::HandleSetWorkSchedulerConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string configData;
    if (!data.ReadString(configData)) {
        WS_HILOGE("HandleSetWorkSchedulerConfig read parce configData error");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t sourceType;
    if (!data.ReadInt32(sourceType)) {
        WS_HILOGE("HandleSetWorkSchedulerConfig read parce sourceType error");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t ret = SetWorkSchedulerConfig(configData, sourceType);
    if (!reply.WriteInt32(ret)) {
        WS_HILOGE("HandleSetWorkSchedulerConfig write result failed, Errcode=%{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}

int32_t WorkSchedServiceStub::HandleRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::START_WORK): {
            int32_t ret = StartWorkStub(data);
            reply.WriteInt32(ret);
            return ret;
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_WORK): {
            int32_t ret = StopWorkStub(data);
            reply.WriteInt32(ret);
            return ret;
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_AND_CANCEL_WORK): {
            int32_t ret = StopAndCancelWorkStub(data);
            reply.WriteInt32(ret);
            return ret;
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_AND_CLEAR_WORKS): {
            int32_t ret = StopAndClearWorksStub(data);
            reply.WriteInt32(ret);
            return ret;
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::IS_LAST_WORK_TIMEOUT): {
            return HandleIsLastWorkTimeOutRequest(data, reply);
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::OBTAIN_ALL_WORKS): {
            return HandleObtainAllWorksRequest(data, reply);
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::GET_WORK_STATUS): {
            return HandleGetWorkStatusRequest(data, reply);
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::GET_ALL_RUNNING_WORKS): {
            return HandleGetAllRunningWorksRequest(reply);
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::PAUSE_RUNNING_WORKS): {
            return PauseRunningWorksStub(data, reply);
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::RESUME_PAUSED_WORKS): {
            return ResumePausedWorksStub(data, reply);
        }
        case static_cast<int32_t>(IWorkSchedServiceInterfaceCode::SET_WORK_SCHEDULER_CONFIG): {
            return HandleSetWorkSchedulerConfig(data, reply);
        }
        default: {
            WS_HILOGD("OnRemoteRequest switch default, code: %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    WS_HILOGD("cmd = %{public}u, flags = %{public}d", code, option.GetFlags());
    std::u16string descriptor = WorkSchedServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        WS_HILOGE("failed, descriptor is not matched!");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t idTimer = SetTimer(code);
    int32_t result = HandleRequest(code, data, reply, option);
    CancelTimer(idTimer);
    return result;
}

int32_t WorkSchedServiceStub::StartWorkStub(MessageParcel& data)
{
    sptr<WorkInfo> workInfo = data.ReadStrongParcelable<WorkInfo>();
    if (workInfo == nullptr) {
        WS_HILOGD("workInfo is nullptr");
        return E_PARCEL_OPERATION_FAILED;
    }
    return StartWork(*workInfo);
}

int32_t WorkSchedServiceStub::StopWorkStub(MessageParcel& data)
{
    sptr<WorkInfo> workInfo = data.ReadStrongParcelable<WorkInfo>();
    if (workInfo == nullptr) {
        WS_HILOGD("workInfo is nullptr");
        return E_PARCEL_OPERATION_FAILED;
    }
    return StopWork(*workInfo);
}

int32_t WorkSchedServiceStub::StopAndCancelWorkStub(MessageParcel& data)
{
    sptr<WorkInfo> workInfo = data.ReadStrongParcelable<WorkInfo>();
    if (workInfo == nullptr) {
        WS_HILOGD("workInfo is nullptr");
        return E_PARCEL_OPERATION_FAILED;
    }
    return StopAndCancelWork(*workInfo);
}

int32_t WorkSchedServiceStub::StopAndClearWorksStub(MessageParcel& data)
{
    return StopAndClearWorks();
}

int32_t WorkSchedServiceStub::IsLastWorkTimeoutStub(MessageParcel& data, bool &result)
{
    int32_t workId = data.ReadInt32();
    return IsLastWorkTimeout(workId, result);
}

int32_t WorkSchedServiceStub::ObtainAllWorksStub(MessageParcel& data, std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    return ObtainAllWorks(workInfos);
}

int32_t WorkSchedServiceStub::GetWorkStatusStub(MessageParcel& data, std::shared_ptr<WorkInfo>& workInfo)
{
    int32_t workId;
    READ_PARCEL_WITHOUT_RET(data, Int32, workId);
    return GetWorkStatus(workId, workInfo);
}

int32_t WorkSchedServiceStub::GetAllRunningWorksStub(std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    return GetAllRunningWorks(workInfos);
}

int32_t WorkSchedServiceStub::PauseRunningWorksStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t uid = -1;
    if (!data.ReadInt32(uid)) {
        WS_HILOGE("PauseRunningWorksStub failed, read uid error");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t ret = PauseRunningWorks(uid);
    if (!reply.WriteInt32(ret)) {
        WS_HILOGE("PauseRunningWorksStub failed, write result error");
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}

int32_t WorkSchedServiceStub::ResumePausedWorksStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t uid = -1;
    if (!data.ReadInt32(uid)) {
        WS_HILOGE("ResumePausedWorksStub failed, read uid error");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t ret = ResumePausedWorks(uid);
    if (!reply.WriteInt32(ret)) {
        WS_HILOGE("ResumePausedWorksStub failed, write result error");
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}

int32_t WorkSchedServiceStub::SetTimer(uint32_t code)
{
#ifdef HICOLLIE_ENABLE
    int32_t idTimer = HiviewDFX::INVALID_ID;
    std::map<uint32_t, std::string>::iterator itCollieId = collieCodeStringMap_.find(code);
    if (itCollieId != collieCodeStringMap_.end()) {
        std::string collieStr = itCollieId->second;
        std::string collieName = "WorkSchedulerServiceStub:" + collieStr;
        unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_RECOVERY;
        auto TimerCallback = [collieStr](void *) {
            WS_HILOGE("OnRemoteRequest timeout func: %{public}s", collieStr.c_str());
        };
        idTimer = HiviewDFX::XCollie::GetInstance().SetTimer(
            collieName, XCOLLIE_TIMEOUT_SECONDS, TimerCallback, nullptr, flag);
        WS_HILOGD("SetTimer id: %{public}d, name: %{public}s.", idTimer, collieName.c_str());
    }
    return idTimer;
#else
    WS_HILOGD("No HICOLLIE_ENABLE");
    return -1;
#endif
}

void WorkSchedServiceStub::CancelTimer(int32_t id)
{
#ifdef HICOLLIE_ENABLE
    if (id == HiviewDFX::INVALID_ID) {
        return;
    }
    WS_HILOGD("CancelTimer id: %{public}d.", id);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
#else
    return;
#endif
}
} // namespace WorkScheduler
} // namespace OHOS