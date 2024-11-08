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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICE_STUB_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICE_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>
#include <list>
#include <memory>

#include "iwork_sched_service.h"
#include "iwork_sched_service_ipc_interface_code.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedServiceStub : public IRemoteStub<IWorkSchedService> {
public:
    WorkSchedServiceStub() = default;
    virtual ~WorkSchedServiceStub() = default;
    DISALLOW_COPY_AND_MOVE(WorkSchedServiceStub);

    /**
     * @brief The OnRemoteRequest callback.
     *
     * @param code The code.
     * @param data The data.
     * @param reply The reply.
     * @param option The option.
     * @return ERR_OK on success, others on failure.
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * @brief Start work stub.
     *
     * @param data The data.
     * @return error code, ERR_OK if success.
     */
    int32_t StartWorkStub(MessageParcel& data);
    /**
     * @brief Stop work stub.
     *
     * @param data The data.
     * @return error code, ERR_OK if success.
     */
    int32_t StopWorkStub(MessageParcel& data);
    /**
     * @brief Stop and cancel work stub.
     *
     * @param data The data.
     * @return error code, ERR_OK if success.
     */
    int32_t StopAndCancelWorkStub(MessageParcel& data);
    /**
     * @brief Stop and clear works stub.
     *
     * @param data The data.
     * @return error code, ERR_OK if success.
     */
    int32_t StopAndClearWorksStub(MessageParcel& data);
    /**
     * @brief The last work time out stub.
     *
     * @param data The data.
     * @param result True if the work executed time out, else false.
     * @return error code, ERR_OK if success.
     */
    int32_t IsLastWorkTimeoutStub(MessageParcel& data, bool &result);
    /**
     * @brief Obtain all works stub.
     *
     * @param data The data.
     * @param workInfos The infos of work.
     * @return error code, ERR_OK if success.
     */
    int32_t ObtainAllWorksStub(MessageParcel& data, std::list<std::shared_ptr<WorkInfo>>& workInfos);
    /**
     * @brief Get work status stub.
     *
     * @param data The data.
     * @param workInfo The info of work.
     * @return error code, ERR_OK if success.
     */
    int32_t GetWorkStatusStub(MessageParcel& data, std::shared_ptr<WorkInfo>& workInfo);

    /**
     * @brief Get the Running Work Scheduler Work object
     *
     * @param workInfos The infos of work.
     * @return ErrCode ERR_OK on success, others on failure
     */
    int32_t GetAllRunningWorksStub(std::list<std::shared_ptr<WorkInfo>>& workInfos);

    /**
     * @brief Pause Running Works.
     *
     * @param data The data.
     * @param reply The reply.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    int32_t PauseRunningWorksStub(MessageParcel &data, MessageParcel& reply);

    /**
     * @brief Resume Paused works.
     *
     * @param data The data.
     * @param reply The reply.
     * @return ErrCode ERR_OK on success, others on failure
     */
    int32_t ResumePausedWorksStub(MessageParcel &data, MessageParcel& reply);
private:
    int32_t HandleObtainAllWorksRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetWorkStatusRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllRunningWorksRequest(MessageParcel &reply);
    int32_t HandleIsLastWorkTimeOutRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t HandleSetWorkSchedulerConfig(MessageParcel &data, MessageParcel &reply);
    int32_t SetTimer(uint32_t code);
    void CancelTimer(int32_t id);
private:
    std::map<uint32_t, std::string> collieCodeStringMap_ = {
        { uint32_t(IWorkSchedServiceInterfaceCode::START_WORK), "START_WORK" },
    };
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICE_STUB_H