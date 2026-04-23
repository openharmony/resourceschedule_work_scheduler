## UTILS 模块

### 功能概述

公共工具类，提供日志、错误码、常量等基础支持，为 Work Scheduler 服务和客户端提供统一的底层设施。

### 子目录分布

```
native/
├── include/     # 头文件定义
└── src/         # 实现代码
```

### 核心文件说明

| 文件 | 说明 |
|------|------|
| work_sched_hilog.h | 日志宏定义，支持 F/E/W/I/D 五级日志 |
| work_sched_errors.h | 错误码定义，包含服务错误码和参数错误码两类 |
| work_sched_constants.h | 常量定义，包括运行限制、超时时间、事件码等 |
| work_sched_utils.h | 工具函数，提供账户ID获取、路径转换、时间获取等 |
| work_sched_common.h | 公共类型定义 |
| work_sched_system_policy.h | 系统策略相关定义 |
| work_sched_hisysevent_report.h | 系统事件上报 |

### 使用规范

#### 日志使用
```cpp
#include "work_sched_hilog.h"

WS_HILOGD("Debug: %{public}d", value);    // 调试日志
WS_HILOGI("Info: %{public}s", str);       // 信息日志
WS_HILOGW("Warning");                      // 警告日志
WS_HILOGE("Error: %{public}d", code);     // 错误日志
WS_HILOGF("Fatal: %{public}s", msg);      // 致命错误日志
```

使用 `%{public}` 标记公开数据，敏感数据不使用该标记。

#### 错误码使用
```cpp
#include "work_sched_errors.h"

// 服务错误码
E_PERMISSION_DENIED        // 201 - 权限被拒绝
E_PARAM_ERROR              // 401 - 参数错误
E_CHECK_WORKINFO_FAILED    // 970000401 - WorkInfo 检查失败
E_ADD_REPEAT_WORK_ERR      // 970000501 - 重复添加延迟任务
E_INNER_ERR                // 内部错误

// 参数错误码
E_WORKID_ERR               // workId 参数错误
E_BUNDLE_OR_ABILITY_NAME_ERR  // 包名/Ability名错误
```

#### 常量使用
```cpp
#include "work_sched_constants.h"

MAX_RUNNING_COUNT          // 最大同时运行任务数 (3)
MAX_WORK_COUNT_PER_UID    // 每UID最大任务数 (10)
WATCHDOG_TIME             // 看门狗超时时间 (2分钟)
TIME_CYCLE                // 时间周期 (10分钟)
```

#### 工具函数使用
```cpp
#include "work_sched_utils.h"

int32_t accountId = WorkSchedUtils::GetCurrentAccountId();
int32_t userId = WorkSchedUtils::GetUserIdByUid(uid);
uint64_t timeMs = WorkSchedUtils::GetCurrentTimeMs();
bool isSystem = WorkSchedUtils::IsSystemApp();
```