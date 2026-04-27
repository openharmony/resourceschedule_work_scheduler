# FRAMEWORKS 模块

## 功能概述

客户端框架实现，提供延迟任务客户端接口和扩展能力。

## 子目录分布

- **include/**：公共头文件（work_info.h、workscheduler_srv_client.h、work_condition.h）
- **src/**：客户端实现
- **extension/**：WorkSchedulerExtension 扩展

## 核心类说明

### WorkInfo

延迟任务信息封装类，继承自 Parcelable，支持 IPC 序列化。

**成员变量说明：**

| 变量名 | 类型 | 默认值 | 含义与作用 |
|--------|------|--------|------------|
| workId_ | int32_t | INVALID_VALUE | **延迟任务ID**，应用内唯一标识，必填。用于区分不同的延迟任务实例。 |
| bundleName_ | std::string | 空 | **包名**，必填。标识延迟任务所属的应用包，系统据此定位任务归属。 |
| abilityName_ | std::string | 空 | **Ability名**，必填。标识任务执行时拉起的具体 Ability，系统通过此名称启动对应组件。 |
| uid_ | int32_t | INVALID_VALUE | **用户ID**，系统填充。标识任务所属应用的用户身份，用于权限校验和任务隔离。 |
| persisted_ | bool | false | **是否持久化**。设置为 true 时，任务信息会持久化存储，设备重启后自动恢复执行。 |
| conditionMap_ | std::map | 空 | **条件映射**，存储任务执行条件。键为 WorkCondition::Type，值为 Condition 结构体。支持的类型：NETWORK、CHARGER、BATTERY_STATUS、BATTERY_LEVEL、STORAGE、TIMER、DEEP_IDLE。 |
| extras_ | WantParams | nullptr | **额外参数**，携带任务执行时的自定义参数。支持 number、string、bool 类型，传递给目标 Ability。 |
| callBySystemApp_ | bool | false | **是否由系统应用调用**。标记任务是否由系统应用发起，影响执行优先级和权限策略。 |
| preinstalled_ | bool | false | **是否预安装任务**。标记任务是否来自预安装配置文件，预安装任务享有特殊调度策略。 |
| uriKey_ | std::string | 空 | **URI键**，用于数据共享场景的任务标识，关联 DataShare URI。 |
| appIndex_ | int32_t | 0 | **应用索引**，多用户场景下的应用实例索引，区分同一应用的不同用户实例。 |
| saId_ | int32_t | INVALID_VALUE | **系统能力ID**，标识任务关联的系统服务 SA，用于 SA 触发的延迟任务。 |
| residentSa_ | bool | false | **是否常驻SA**。标记关联的 SA 是否为常驻服务，影响进程拉起策略。 |
| isInnerApply_ | bool | false | **是否内部申请**。标记任务是否由系统内部模块发起，跳过常规校验流程。 |
| earliestStartTime_ | int32_t | 0 | **最早开始时间**，延迟任务可执行的最早时间点（毫秒），用于延迟触发场景。 |
| createTime_ | uint64_t | 当前时间 | **创建时间**，任务创建的时间戳（毫秒），用于任务生命周期管理。 |
| deepIdleTime_ | int32_t | 0 | **深度空闲时间**，设备进入深度空闲状态后的等待时间（毫秒），用于空闲触发场景。 |
| triggerType_ | int32_t | UNKNOWN | **触发类型**，标识任务的触发方式（WorkCondition::Type），如 NETWORK、TIMER 等。 |

**条件类型（conditionMap_ 支持的类型）：**

| 条件类型 | 枚举值 | 说明 | Condition 字段使用 |
|----------|--------|------|-------------------|
| NETWORK | 0 | 网络类型条件 | enumVal 存储网络类型枚举 |
| CHARGER | 1 | 充电类型条件 | boolVal 存储是否充电，enumVal 存储充电类型 |
| BATTERY_STATUS | 2 | 电池状态条件 | enumVal 存储电池状态枚举 |
| BATTERY_LEVEL | 3 | 电池电量条件 | intVal 存储电量阈值 |
| STORAGE | 4 | 存储状态条件 | enumVal 存储存储状态枚举 |
| TIMER | 5 | 定时器条件 | uintVal 存储间隔时间，intVal 存储循环次数，boolVal 存储是否重复 |
| DEEP_IDLE | 6 | 深度空闲条件 | enumVal 存储深度空闲状态 |
| GROUP | 7 | 应用分组条件 | intVal 存储新分组，strVal 存储包名 |

**主要方法：**
- `SetWorkId/GetWorkId`：设置/获取任务ID
- `SetElement`：设置包名和Ability名
- `RequestNetworkType/GetNetworkType`：网络类型条件
- `RequestChargerType/GetChargerType`：充电类型条件
- `RequestBatteryLevel/GetBatteryLevel`：电池电量条件
- `RequestBatteryStatus/GetBatteryStatus`：电池状态条件
- `RequestStorageLevel/GetStorageLevel`：存储状态条件
- `RequestRepeatCycle`：重复周期设置
- `Marshalling/Unmarshalling`：IPC 序列化
- `ParseToJsonStr/ParseFromJson`：JSON 序列化

### WorkSchedulerSrvClient

服务客户端代理类，单例模式，封装与 WorkSchedulerService 的 IPC 通信。

**主要接口：**
| 方法 | 说明 |
|------|------|
| StartWork | 启动延迟任务 |
| StopWork | 停止延迟任务 |
| StopAndCancelWork | 停止并取消延迟任务 |
| StopAndClearWorks | 停止并清除所有延迟任务 |
| IsLastWorkTimeout | 检查上次任务是否超时 |
| GetWorkStatus | 获取任务状态 |
| ObtainAllWorks | 获取所有延迟任务 |
| GetAllRunningWorks | 获取运行中任务 |
| PauseRunningWorks | 暂停运行中任务 |
| ResumePausedWorks | 恢复暂停任务 |

### WorkCondition

条件类型定义，包含枚举和条件结构体。

**Type 枚举：** NETWORK、CHARGER、BATTERY_STATUS、BATTERY_LEVEL、STORAGE、TIMER、GROUP、DEEP_IDLE、STANDBY、UNKNOWN

**Network 枚举：** NETWORK_TYPE_ANY、NETWORK_TYPE_MOBILE、NETWORK_TYPE_WIFI、NETWORK_TYPE_BLUETOOTH、NETWORK_TYPE_WIFI_P2P、NETWORK_TYPE_ETHERNET

**Charger 枚举：** CHARGING_PLUGGED_ANY、CHARGING_PLUGGED_AC、CHARGING_PLUGGED_USB、CHARGING_PLUGGED_WIRELESS、CHARGING_UNPLUGGED

**Condition 结构体：** 包含 enumVal、intVal、uintVal、boolVal、strVal、timeVal 字段

## WorkSchedulerExtension 说明

### WorkSchedulerExtension

延迟任务扩展基类，继承自 `ExtensionBase<WorkSchedulerExtensionContext>`。

**生命周期回调：**
- `OnWorkStart(WorkInfo&)`：任务开始执行回调
- `OnWorkStop(WorkInfo&)`：任务停止执行回调

### WorkSchedulerExtensionContext

扩展上下文类，继承自 ExtensionContext。

**接口：**
- `StartServiceExtensionAbility`：启动服务扩展
- `StopServiceExtensionAbility`：停止服务扩展