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
#include "work_sched_service_proxy.h"

#include <ipc_types.h>
#include <message_parcel.h>
#include "iwork_sched_service_ipc_interface_code.h"
#include "work_sched_errors.h"

#include "work_sched_common.h"

namespace OHOS {
namespace WorkScheduler {
int32_t WorkSchedServiceProxy::StartWork(WorkInfo& workInfo)
{
    WS_HILOGI("proxy Call StartWork come in");
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!data.WriteParcelable(&workInfo)) {
        WS_HILOGE("StartWork failed, write workinfo failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::START_WORK), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, error code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t result = E_PARCEL_OPERATION_FAILED;
    READ_PARCEL_WITHOUT_RET(reply, Int32, result);
    WS_HILOGD("after result : %{public}s ", std::to_string(result).c_str());
    return result;
}

int32_t WorkSchedServiceProxy::StopWork(WorkInfo& workInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE(" write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!data.WriteParcelable(&workInfo)) {
        WS_HILOGE("StopWork failed, write workinfo failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_WORK), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t result = E_PARCEL_OPERATION_FAILED;
    READ_PARCEL_WITHOUT_RET(reply, Int32, result);
    return result;
}

int32_t WorkSchedServiceProxy::StopAndCancelWork(WorkInfo& workInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return false;
    }

    if (!data.WriteParcelable(&workInfo)) {
        WS_HILOGE("StopAndCancelWork failed, write workinfo failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_AND_CANCEL_WORK),
        data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t result = E_PARCEL_OPERATION_FAILED;
    READ_PARCEL_WITHOUT_RET(reply, Int32, result);
    return result;
}

int32_t WorkSchedServiceProxy::StopAndClearWorks()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_AND_CLEAR_WORKS),
        data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t result = E_PARCEL_OPERATION_FAILED;
    READ_PARCEL_WITHOUT_RET(reply, Int32, result);
    return result;
}

int32_t WorkSchedServiceProxy::IsLastWorkTimeout(int32_t workId, bool &result)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_MEMORY_OPERATION_FAILED);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    WRITE_PARCEL_WITHOUT_RET(data, Int32, workId);
    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::IS_LAST_WORK_TIMEOUT),
        data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    ret = E_PARCEL_OPERATION_FAILED;
    READ_PARCEL_WITHOUT_RET(reply, Int32, ret);
    READ_PARCEL_WITHOUT_RET(reply, Bool, result);
    return ret;
}

int32_t WorkSchedServiceProxy::ObtainAllWorks(std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_MEMORY_OPERATION_FAILED);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::OBTAIN_ALL_WORKS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }

    ErrCode errCode;
    READ_PARCEL_WITHOUT_RET(reply, Int32, errCode);
    int32_t worksize = 0;
    READ_PARCEL_WITHOUT_RET(reply, Int32, worksize);
    WS_HILOGD("ObtainAllWorks worksize from read parcel is: %{public}d", worksize);
    for (int32_t i = 0; i < worksize; i++) {
        sptr<WorkInfo> workInfo = reply.ReadStrongParcelable<WorkInfo>();
        if (workInfo == nullptr) {
            continue;
        }
        WS_HILOGD("WP read from parcel, workInfo ID: %{public}d", workInfo->GetWorkId());
        workInfos.emplace_back(std::make_shared<WorkInfo>(*workInfo));
    }
    WS_HILOGD("return list size: %{public}zu", workInfos.size());
    return errCode;
}

int32_t WorkSchedServiceProxy::GetWorkStatus(int32_t &workId, std::shared_ptr<WorkInfo>& workInfo)
{
    WS_HILOGD("enter, workId: %{public}d", workId);
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_MEMORY_OPERATION_FAILED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }
    WRITE_PARCEL_WITHOUT_RET(data, Int32, workId);
    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::GET_WORK_STATUS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }
    ErrCode errCode;
    READ_PARCEL_WITHOUT_RET(reply, Int32, errCode);
    sptr<WorkInfo> workInfoSptr = reply.ReadStrongParcelable<WorkInfo>();
    if (workInfoSptr == nullptr) {
        return E_WORK_NOT_EXIST_FAILED;
    }
    workInfo = std::make_shared<WorkInfo>(*workInfoSptr);
    return errCode;
}

int32_t WorkSchedServiceProxy::GetAllRunningWorks(std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    WS_HILOGD("get all running work sheduler work");
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_MEMORY_OPERATION_FAILED);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::GET_ALL_RUNNING_WORKS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }

    ErrCode errCode;
    READ_PARCEL_WITHOUT_RET(reply, Int32, errCode);
    int32_t worksize = 0;
    READ_PARCEL_WITHOUT_RET(reply, Int32, worksize);
    WS_HILOGD("GetAllRunningWorks worksize from read parcel is: %{public}d", worksize);
    for (int32_t i = 0; i < worksize; i++) {
        sptr<WorkInfo> workInfo = reply.ReadStrongParcelable<WorkInfo>();
        if (workInfo == nullptr) {
            continue;
        }
        WS_HILOGD("WP read from parcel, workInfo ID: %{public}d", workInfo->GetWorkId());
        workInfos.emplace_back(std::make_shared<WorkInfo>(*workInfo));
    }
    WS_HILOGD("return list size: %{public}zu", workInfos.size());
    return errCode;
}

int32_t WorkSchedServiceProxy::PauseRunningWorks(int32_t uid)
{
    WS_HILOGD("Pause Running Work Scheduler Work, uid:%{public}d", uid);
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_CLIENT_CONNECT_SERVICE_FAILED);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!data.WriteInt32(uid)) {
        WS_HILOGE("PauseRunningWorks failed, write uid failed!");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::PAUSE_RUNNING_WORKS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!reply.ReadInt32(ret)) {
        WS_HILOGE("PauseRunningWorks failed, read errCode error");
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}

int32_t WorkSchedServiceProxy::ResumePausedWorks(int32_t uid)
{
    WS_HILOGD("Resume Paused Work Scheduler Work, uid:%{public}d", uid);
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_CLIENT_CONNECT_SERVICE_FAILED);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("ResumePausedWorks failed, write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!data.WriteInt32(uid)) {
        WS_HILOGE("ResumePausedWorks failed, write uid failed!");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::RESUME_PAUSED_WORKS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!reply.ReadInt32(ret)) {
        WS_HILOGE("ResumePausedWorks failed, read errCode error");
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}

int32_t WorkSchedServiceProxy::SetWorkSchedulerConfig(const std::string &configData, int32_t sourceType)
{
    WS_HILOGD("Set work scheduler config");
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, E_CLIENT_CONNECT_SERVICE_FAILED);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("SetWorkSchedulerConfig failed, write descriptor failed!");
        return E_PARCEL_OPERATION_FAILED;
    }
    if (!data.WriteString(configData)) {
        WS_HILOGE("SetWorkSchedulerConfig failed, write configData failed!");
        return E_PARCEL_OPERATION_FAILED;
    }
    if (!data.WriteInt32(sourceType)) {
        WS_HILOGE("SetWorkSchedulerConfig failed, write sourceType failed!");
        return E_PARCEL_OPERATION_FAILED;
    }
    int32_t ret = remote->SendRequest(
        static_cast<int32_t>(IWorkSchedServiceInterfaceCode::SET_WORK_SCHEDULER_CONFIG), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendRequest is failed, err code: %{public}d", ret);
        return E_PARCEL_OPERATION_FAILED;
    }

    if (!reply.ReadInt32(ret)) {
        WS_HILOGE("SetWorkSchedulerConfig failed, read errCode error");
        return E_PARCEL_OPERATION_FAILED;
    }
    return ret;
}
} // namespace WorkScheduler
} // namespace OHOS
