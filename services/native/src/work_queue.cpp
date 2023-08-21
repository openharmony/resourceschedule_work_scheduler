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
#include <set>
#include "work_queue.h"

#include "work_condition.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "work_scheduler_service.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
vector<shared_ptr<WorkStatus>> WorkQueue::OnConditionChanged(WorkCondition::Type type,
    shared_ptr<DetectorValue> conditionVal)
{
    shared_ptr<Condition> value = ParseCondition(type, conditionVal);
    vector<shared_ptr<WorkStatus>> result;
    std::set<int32_t> uidList;
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    for (auto it : workList_) {
        if (it->OnConditionChanged(type, value) == E_GROUP_CHANGE_NOT_MATCH_HAP) {
            continue;
        }
        if (uidList.count(it->uid_) > 0 && it->GetMinInterval() != 0 &&
            !DelayedSpSingleton<WorkSchedulerService>::GetInstance()->CheckEffiResApplyInfo(it->uid_)) {
            WS_HILOGI("One uid can start only one work.");
            continue;
        }
        if (it->IsReady()) {
            result.emplace_back(it);
            uidList.insert(it->uid_);
        } else {
            if (it->IsReadyStatus()) {
                it->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
            }
        }
        if (it->needRetrigger_) {
            result.emplace_back(it);
        }
    }
    return result;
}

shared_ptr<Condition> WorkQueue::ParseCondition(WorkCondition::Type type,
    shared_ptr<DetectorValue> conditionVal)
{
    shared_ptr<Condition> value = make_shared<Condition>();
    switch (type) {
        case WorkCondition::Type::NETWORK:
        // fall-through
        case WorkCondition::Type::BATTERY_STATUS:
        // fall-through
        case WorkCondition::Type::STORAGE: {
            value->enumVal = conditionVal->intVal;
            break;
        }
        case WorkCondition::Type::CHARGER: {
            value->enumVal = conditionVal->intVal;
            value->boolVal = (conditionVal->intVal > WorkCondition::Charger::CHARGING_PLUGGED_ANY
                && conditionVal->intVal < WorkCondition::Charger::CHARGING_UNKNOWN);
            break;
        }
        case WorkCondition::Type::BATTERY_LEVEL: {
            value->intVal = conditionVal->intVal;
            break;
        }
        case WorkCondition::Type::TIMER: {
            break;
        }
        case WorkCondition::Type::GROUP: {
            value->enumVal = conditionVal->intVal;
            value->intVal = conditionVal->timeVal;
            value->boolVal = conditionVal->boolVal;
            value->strVal = conditionVal->strVal;
            break;
        }
        case WorkCondition::Type::STANDBY: {
            value->boolVal = conditionVal->boolVal;
            break;
        }
        default: {}
    }
    return value;
}

void WorkQueue::Push(shared_ptr<vector<shared_ptr<WorkStatus>>> workStatusVector)
{
    for (auto it : *workStatusVector) {
        Push(it);
    }
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    workList_.sort(WorkComp());
}

void WorkQueue::Push(shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    if (this->Contains(make_shared<string>(workStatus->workId_))) {
        for (auto it : workList_) {
            if (it->workId_.compare(workStatus->workId_) == 0) {
                return;
            }
        }
        return;
    }
    workList_.push_back(workStatus);
}

bool WorkQueue::Remove(shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    auto iter = std::find(workList_.cbegin(), workList_.cend(), workStatus);
    if (iter != workList_.end()) {
        workList_.remove(*iter);
    }
    return true;
}

uint32_t WorkQueue::GetSize()
{
    return workList_.size();
}

bool WorkQueue::Contains(std::shared_ptr<std::string> workId)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    auto iter = std::find_if(workList_.cbegin(), workList_.cend(), [&workId]
        (const shared_ptr<WorkStatus> &workStatus) { return workId->compare(workStatus->workId_) == 0; });
    if (iter != workList_.end()) {
        return true;
    }
    return false;
}

shared_ptr<WorkStatus> WorkQueue::Find(string workId)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    auto iter = std::find_if(workList_.cbegin(), workList_.cend(),
        [&workId](const shared_ptr<WorkStatus> &workStatus) { return workStatus->workId_ == workId; });
    if (iter != workList_.end()) {
        return *iter;
    }
    return nullptr;
}

shared_ptr<WorkStatus> WorkQueue::GetWorkToRunByPriority()
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    workList_.sort(WorkComp());
    auto work = workList_.begin();
    shared_ptr<WorkStatus> workStatus = nullptr;
    while (work != workList_.end()) {
        if ((*work)->GetStatus() == WorkStatus::CONDITION_READY) {
            workStatus = *work;
            break;
        }
        work++;
    }
    return workStatus;
}

bool WorkQueue::CancelWork(shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    workList_.remove(workStatus);
    return true;
}

list<shared_ptr<WorkStatus>> WorkQueue::GetWorkList()
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    return workList_;
}

void WorkQueue::RemoveUnReady()
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    workList_.remove_if([](shared_ptr<WorkStatus> value) {
        return (value->GetStatus() != WorkStatus::Status::CONDITION_READY);
    });
}

int32_t WorkQueue::GetRunningCount()
{
    int32_t count = 0;
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    for (shared_ptr<WorkStatus> work : workList_) {
        if (work->IsRunning()) {
            count++;
        }
    }
    return count;
}

std::list<std::shared_ptr<WorkInfo>> WorkQueue::GetRunningWorks()
{
    std::list<std::shared_ptr<WorkInfo>> workInfo;
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    for (shared_ptr<WorkStatus> work : workList_) {
        if (work->IsRunning()) {
            auto info = std::make_shared<WorkInfo>();
            info->SetElement(work->bundleName_, work->abilityName_);
            info->RefreshUid(work->uid_);
            workInfo.emplace_back(info);
        }
    }
    return workInfo;
}

void WorkQueue::GetWorkIdStr(string& result)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    for (auto it : workList_) {
        result.append(it->workId_ + ", ");
    }
}

void WorkQueue::Dump(string& result)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    for (auto it : workList_) {
        it->Dump(result);
    }
}

void WorkQueue::ClearAll()
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    workList_.clear();
}

bool WorkComp::operator () (const shared_ptr<WorkStatus> w1, const shared_ptr<WorkStatus> w2)
{
    return w1->priority_ >= w2->priority_;
}

void WorkQueue::SetMinIntervalByDump(int64_t interval)
{
    std::lock_guard<std::recursive_mutex> lock(workListMutex_);
    for (auto it : workList_) {
        it->SetMinIntervalByDump(interval);
    }
}
} // namespace WorkScheduler
} // namespace OHOS