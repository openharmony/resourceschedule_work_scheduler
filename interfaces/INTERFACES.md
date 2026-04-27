# INTERFACES 模块

## 功能概述

公共 API 定义，支持多语言绑定（JS/NAPI、仓颉、ArkTS/Taihe），为上层应用提供延迟任务调度接口。

## 子目录分布

```
interfaces/kits/
├── js/napi/             # JavaScript/NAPI 绑定
│   ├── src/             # API 实现
│   ├── include/         # 头文件
│   └── work_scheduler_extension/        # 扩展能力
│       └── work_scheduler_extension_context/  # 扩展上下文
├── cj/work_scheduler/   # 仓颉（Cangjie）语言绑定
│   ├── work_scheduler_ffi.h    # FFI 接口定义
│   └── work_scheduler_ffi.cpp  # FFI 实现
└── ets/taihe/           # ArkTS/Taihe 绑定
    ├── work_scheduler/          # 核心 API
    │   └── idl/                 # IDL 接口定义
    └── work_scheduler_extension/ # 扩展能力
```

## JS/NAPI 绑定说明

### 核心 API（init.cpp）
| 方法 | 说明 |
|------|------|
| `startWork` | 启动延迟任务 |
| `stopWork` | 停止延迟任务 |
| `getWorkStatus` | 获取延迟任务状态 |
| `obtainAllWorks` | 获取所有延迟任务 |
| `stopAndClearWorks` | 停止并清除所有延迟任务 |
| `isLastWorkTimeOut` | 检查上次延迟任务是否超时 |

### 枚举常量
- **NetworkType**: NETWORK_TYPE_ANY/MOBILE/WIFI/BLUETOOTH/WIFI_P2P/ETHERNET
- **ChargingType**: CHARGING_PLUGGED_ANY/AC/USB/WIRELESS
- **BatteryStatus**: BATTERY_STATUS_LOW/OKAY/LOW_OR_OKAY
- **StorageRequest**: STORAGE_LEVEL_LOW/OKAY/LOW_OR_OKAY

### 扩展能力
- `WorkSchedulerExtensionAbility`: 延迟任务扩展能力基类
- `WorkSchedulerExtensionContext`: 延迟任务扩展上下文

## 仓颉绑定说明

`cj/work_scheduler/` 下 FFI 接口：
- `CJ_StartWork` / `CJ_StartWorkV2`: 启动延迟任务
- `CJ_StopWork` / `CJ_StopWorkV2`: 停止延迟任务
- `CJ_GetWorkStatus` / `CJ_GetWorkStatusV2`: 获取延迟任务状态
- `CJ_ObtainAllWorks` / `CJ_ObtainAllWorksV2`: 获取所有延迟任务
- `CJ_IsLastWorkTimeOut`: 检查超时
- `CJ_StopAndClearWorks`: 清除所有延迟任务

数据结构：`RetWorkInfo` / `RetWorkInfoV2` 包含 workId、bundleName、abilityName 及各类条件参数。

## Taihe 绑定说明

`ets/taihe/work_scheduler/idl/ohos.resourceschedule.workScheduler.taihe` 定义：

### IDL 接口
- `StartWork(work: WorkInfo)`: 启动延迟任务
- `StopWork(work: WorkInfo, needCancel?)`: 停止延迟任务
- `GetWorkStatusSync(workId)`: 同步获取状态（支持异步/Promise）
- `ObtainAllWorksSync()`: 同步获取所有延迟任务（支持异步/Promise）
- `StopAndClearWorks()`: 清除所有延迟任务
- `IsLastWorkTimeOutSync(workId)`: 检查超时（支持异步/Promise）