/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_INFO_H

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <string>

#include <parcel.h>
#include "want_params.h"
#include "refbase.h"

#include "work_condition.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace WorkScheduler {
class WorkInfo : public Parcelable {
public:
    explicit WorkInfo();
    WorkInfo(const WorkInfo& workInfo)
    {
        workId_ = workInfo.workId_;
        bundleName_ = workInfo.bundleName_;
        abilityName_ = workInfo.abilityName_;
        persisted_ = workInfo.persisted_;
        uid_ = workInfo.uid_;
        callBySystemApp_ = workInfo.callBySystemApp_;
        preinstalled_ = workInfo.preinstalled_;
        uriKey_ = workInfo.uriKey_;
        appIndex_ = workInfo.appIndex_;
        extension_ = workInfo.extension_;
        saId_ = workInfo.saId_;
        residentSa_ = workInfo.residentSa_;
        extras_ = workInfo.extras_;
        conditionMap_ = workInfo.conditionMap_;
        isInnerApply_ = workInfo.isInnerApply_;
    }
    ~WorkInfo() override;
    /**
     * @brief Set the id of workId.
     *
     * @param workId The id of work.
     */
    void SetWorkId(int32_t workId);
    /**
     * @brief Set element.
     *
     * @param bundleName The name of bundle.
     * @param abilityName The name of ability
     */
    void SetElement(std::string bundleName, std::string abilityName);
    /**
     * @brief Request persisted.
     *
     * @param persisted The persisted.
     */
    void RequestPersisted(bool persisted);
    /**
     * @brief Request network type.
     *
     * @param condition The condition.
     */
    void RequestNetworkType(WorkCondition::Network condition);
    /**
     * @brief Request charger type.
     *
     * @param isCharging Charging or not.
     * @param condition The condition.
     */
    void RequestChargerType(bool isCharging, WorkCondition::Charger condition);
    /**
     * @brief Request battery level.
     *
     * @param battLevel The battery level.
     */
    void RequestBatteryLevel(int32_t battLevel);
    /**
     * @brief Request battery status.
     *
     * @param condition The condition.
     */
    void RequestBatteryStatus(WorkCondition::BatteryStatus condition);
    /**
     * @brief Request storage level.
     *
     * @param condition The condition.
     */
    void RequestStorageLevel(WorkCondition::Storage condition);
    /**
     * @brief Request repeat cycle.
     *
     * @param timeInterval The time interval.
     * @param cycle The cycle.
     */
    void RequestRepeatCycle(uint32_t timeInterval, int32_t cycle);
    /**
     * @brief Request repeat cycle.
     *
     * @param timeInterval The time interval.
     */
    void RequestRepeatCycle(uint32_t timeInterval);
    /**
     * @brief Request base time and repeat cycle.
     *
     * @param baseTime The base time.
     * @param cycle The cycle.
     */
    void RequestBaseTimeAndCycle(time_t baseTime, int32_t cycle);
    /**
     * @brief Request base time.
     *
     * @param baseTime The base time.
     */
    void RequestBaseTime(time_t baseTime);
    /**
     * @brief Request extra parameters.
     *
     * @param extras extra parameters.
     */
    void RequestExtras(AAFwk::WantParams extras);
    /**
     * @brief Request DeepIdle.
     *
     * @param deepIdle The DeepIdle status.
     */
    void RequestDeepIdle(bool deepIdle);
    /**
     * @brief Refresh uid.
     *
     * @param uid The uid.
     */
    void RefreshUid(int32_t uid);
    /**
     * @brief Refresh appIndex.
     *
     * @param appIndex The appIndex.
     */
    void RefreshAppIndex(int32_t appIndex);
    /**
     * @brief Refresh extension.
     *
     * @param extension The extension.
     */
    void RefreshExtension(bool extension);
    /**
     * @brief Set callBySystemApp flag.
     */
    void SetCallBySystemApp(bool callBySystemApp);
    /**
     * @brief Get uid.
     *
     * @return The uid.
     */
    int32_t GetUid();

    /**
     * @brief Get the id of work.
     *
     * @return The id of work.
     */
    int32_t GetWorkId();
    /**
     * @brief Get the name of bundle.
     *
     * @return The name of bundle.
     */
    std::string GetBundleName();
    /**
     * @brief Get the name of ability.
     *
     * @return The name of ability.
     */
    std::string GetAbilityName();
    /**
     * @brief Check whether the work is persist.
     *
     * @return Persist or not.
     */
    bool IsPersisted();
    /**
     * @brief Get the type of network.
     *
     * @return The type of network.
     */
    WorkCondition::Network GetNetworkType();
    /**
     * @brief Get the type of charger.
     *
     * @return The type of charger.
     */
    WorkCondition::Charger GetChargerType();
    /**
     * @brief Get the level of battery.
     *
     * @return The level of battery.
     */
    int32_t GetBatteryLevel();
    /**
     * @brief Get the status of battery.
     *
     * @return The status of battery.
     */
    WorkCondition::BatteryStatus GetBatteryStatus();
    /**
     * @brief Get the level of storage.
     *
     * @return The level of storage.
     */
    WorkCondition::Storage GetStorageLevel();
    /**
     * @brief Check whether the work is repeat.
     *
     * @return Repeat or not.
     */
    bool IsRepeat();
    /**
     * @brief Get the time interval.
     *
     * @return The time interval.
     */
    uint32_t GetTimeInterval();
    /**
     * @brief Get the count of cycle.
     *
     * @return The count of cycle.
     */
    int32_t GetCycleCount();
    /**
     * @brief Get the base time.
     *
     * @return The base time.
     */
    time_t GetBaseTime();
    /**
     * @brief Get the map of condition.
     *
     * @return The map of condition.
     */
    std::shared_ptr<std::map<WorkCondition::Type, std::shared_ptr<Condition>>> GetConditionMap();
    /**
     * @brief Get extra parameters.
     *
     * @return extra parameters.
     */
    std::shared_ptr<AAFwk::WantParams> GetExtras() const;
    /**
     * @brief Get callBySystemApp flag.
     */
    bool IsCallBySystemApp();
    /**
     * @brief Marshalling.
     *
     * @param parcel The parcel.
     * @return True if success,else false.
     */
    bool Marshalling(Parcel &parcel) const override;
    /**
     * @brief Unmarshalling.
     *
     * @param parcel The parcel.
     * @return Read.
     */
    static WorkInfo* Unmarshalling(Parcel &parcel);
    /**
     * @brief Dump.
     *
     * @param result The result.
     */
    void Dump(std::string &result);
    /**
     * @brief Parse to json str.
     *
     * @return Result.
     */
    std::string ParseToJsonStr();
    /**
     * @brief Parse from json.
     *
     * @param value The value.
     * @return True if success,else false.
     */
    bool ParseFromJson(const nlohmann::json &value);
    /**
     * @brief Parse element from json.
     *
     * @param value The value.
     * @return True if success,else false.
     */
    bool ParseElementFromJson(const nlohmann::json &value);
    /**
     * @brief Set preinstalled flag.
    */
    void SetPreinstalled(bool preinstalled);
    /**
     * @brief Get preinstalled flag.
     */
    bool IsPreinstalled();
    /**
     * @brief Get uri key.
     */
    std::string GetUriKey();
    /**
     * @brief Get deepIdle status.
     */
    WorkCondition::DeepIdle GetDeepIdle();
    /**
     * @brief Get appIndex.
     *
     * @return The appIndex.
     */
    int32_t GetAppIndex() const;
    /**
     * @brief Get extension.
     *
     * @return The extension.
     */
    bool GetExtension() const;
    /**
     * @brief Get saId.
     *
     * @return The saId.
     */
    int32_t GetSaId() const;
    /**
     * @brief Refresh saId.
     *
     * @param saId The saId.
     */
    void RefreshSaId(int32_t saId);
    /**
     * @brief Get the work is residentSa.
     *
     * @return ResidentSa or not.
     */
    bool IsResidentSa() const;
    bool IsSA();
    std::string GetBriefInfo();
    /**
     * @brief Set apply flag.
     */
    void SetIsInnerApply(bool isInnerApply);
    /**
     * @brief Get apply flag.
     *
     * @return The apply flag.
     */
    bool GetIsInnerApply() const;

private:
    int32_t workId_;
    std::string bundleName_;
    std::string abilityName_;
    bool persisted_;
    int32_t uid_;
    std::shared_ptr<AAFwk::WantParams> extras_;
    std::map<WorkCondition::Type, std::shared_ptr<Condition>> conditionMap_;
    bool callBySystemApp_ {false};
    bool preinstalled_ {false};
    std::string uriKey_;
    int32_t appIndex_;
    bool extension_;
    int32_t saId_;
    bool residentSa_;
    bool isInnerApply_ {false};
private:
    static bool UnmarshallCondition(Parcel &parcel, WorkInfo* read, uint32_t mapsize);
    void ParseConditionToJsonStr(nlohmann::json &root);
    void ParseConditionFromJsonStr(const nlohmann::json &value);
    void ParseParametersFromJsonStr(const nlohmann::json &value);
    void ParseTimerFormJsonStr(const nlohmann::json &conditions);
    bool IsHasBoolProp(const nlohmann::json &value, const std::string &key);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_INFO_H