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
#include <string_ex.h>

#include "work_sched_common.h"

namespace OHOS {
namespace WorkScheduler {
bool WorkSchedServiceProxy::StartWork(WorkInfo& workInfo)
{
    WS_HILOGI("proxy Call StartWork come in");
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Parcelable, &workInfo, false);
    
    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::START_WORK), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s SendRequest is failed, error code: %{public}d",
            __func__, ret);
        return false;
    }
    bool result = false;
    READ_PARCEL_WITH_RET(reply, Bool, result, false);
    WS_HILOGD("after result : %{public}s ", std::to_string(result).c_str());
    return result;
}

bool WorkSchedServiceProxy::StopWork(WorkInfo& workInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Parcelable, &workInfo, false);

    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::STOP_WORK), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s SendRequest is failed, err code: %{public}d", __func__, ret);
        return false;
    }
    bool result = false;
    READ_PARCEL_WITH_RET(reply, Bool, result, false);
    return result;
}

bool WorkSchedServiceProxy::StopAndCancelWork(WorkInfo& workInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Parcelable, &workInfo, false);
    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::STOP_AND_CANCEL_WORK), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s SendRequest is failed, err code: %{public}d", __func__, ret);
        return false;
    }
    bool result = false;
    READ_PARCEL_WITH_RET(reply, Bool, result, false);
    return result;
}

bool WorkSchedServiceProxy::StopAndClearWorks()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::STOP_AND_CLEAR_WORKS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s SendRequest is failed, err code: %{public}d", __func__, ret);
        return false;
    }
    bool result = false;
    READ_PARCEL_WITH_RET(reply, Bool, result, false);
    return result;
}

bool WorkSchedServiceProxy::IsLastWorkTimeout(int32_t workId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    WRITE_PARCEL_WITHOUT_RET(data, Int32, workId);
    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::IS_LAST_WORK_TIMEOUT), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s SendRequest is failed, err code: %{public}d", __func__, ret);
        return false;
    }
    bool result = false;
    READ_PARCEL_WITH_RET(reply, Bool, result, false);
    return result;
}

std::list<std::shared_ptr<WorkInfo>> WorkSchedServiceProxy::ObtainAllWorks(int32_t &uid, int32_t &pid)
{
    WS_HILOGD("BUGOAWF function %{public}s invoked.", __func__);
    WS_HILOGD("%{public}s uid: %{public}d, pid: %{public}d", __func__, uid, pid);
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, workInfos);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s write descriptor failed!", __func__);
        return workInfos;
    }

    WRITE_PARCEL_WITHOUT_RET(data, Int32, uid);
    WRITE_PARCEL_WITHOUT_RET(data, Int32, pid);

    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::OBTAIN_ALL_WORKS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("WorkSchedServiceProxy::%{public}s SendRequest is failed, err code: %{public}d",
            __func__, ret);
        return workInfos;
    }

    int32_t worksize = 0;
    READ_PARCEL_WITHOUT_RET(reply, Int32, worksize);
    WS_HILOGD("BUGOAWF WSSP ObtainAllWorks worksize from read parcel is: %{public}d", worksize);
    for (int32_t i = 0; i < worksize; i++) {
        WorkInfo *workInfo = reply.ReadParcelable<WorkInfo>();
        if (workInfo == nullptr) {
            continue;
        }
        WS_HILOGD("WP read from parcel, workInfo ID: %{public}d", workInfo->GetWorkId());
        workInfos.emplace_back(std::make_shared<WorkInfo>(*workInfo));
    }
    WS_HILOGD("WorkSchedServiceProxy::%{public}s return list size: %{public}d", __func__, workInfos.size());
    return workInfos;
}

std::shared_ptr<WorkInfo> WorkSchedServiceProxy::GetWorkStatus(int32_t &uid, int32_t &workId)
{
    WS_HILOGD("%{public}s enter, workId: %{public}d", __func__, workId);
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("%{public}s wirte descriptor failed!", __func__);
        return nullptr;
    }
    WRITE_PARCEL_WITHOUT_RET(data, Int32, uid);
    WRITE_PARCEL_WITHOUT_RET(data, Int32, workId);
    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::GET_WORK_STATUS), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("%{pulbic}s SendRequest is failed, err code: %{public}d", __func__, ret);
        return nullptr;
    }
    WorkInfo *workInfo = reply.ReadParcelable<WorkInfo>();
    if (workInfo == nullptr) {
        return nullptr;
    }
    return std::make_shared<WorkInfo>(*workInfo);
}

bool WorkSchedServiceProxy::ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WorkSchedServiceProxy::GetDescriptor())) {
        WS_HILOGE("%{public}s write descriptor failed!", __func__);
        return false;
    }
    if (!data.WriteStringVector(dumpOption)) {
        WS_HILOGE("[ShellDump] fail: write option failed.");
        return false;
    }
    int ret = remote->SendRequest(static_cast<int>(IWorkSchedService::DUMP_INFO), data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("%{public}s SendRequest is failed, err code: %{public}d", __func__, ret);
        return false;
    }
    if (!reply.ReadBool()) {
        WS_HILOGE("[ShellDump] fail: read result failed.");
        return false;
    }
    if (!reply.ReadStringVector(&dumpInfo)) {
        WS_HILOGE("[ShellDump] fail: read dumpInfo failed.");
        return false;
    }
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS