# AGENTS.md - Work Scheduler 代码库指南

## 项目概述

应用退至后台后，需要执行时效性要求不高的任务，例如有网络时不定期主动获取邮件等，可以使用延迟任务。当应用满足设定的触发条件（包括网络类型、充电类型、存储状态、电池状态、定时状态等）时，将任务添加到执行队列，系统会根据内存、功耗、设备温度、用户使用习惯等统一调度拉起应用，执行相应的延迟任务。

- **子系统**: resourceschedule
- **部件**: work_scheduler
- **语言**: C++（主要），包含 NAPI/JS、CJ（仓颉）和 ETS/Taihe 绑定
- **系统服务 ID**: WORK_SCHEDULE_SERVICE_ID

## 整体目录结构

```
├── frameworks/    # 客户端框架
├── interfaces/    # 公共 API
├── services/      # 服务实现
├── utils/         # 工具类
├── sa_profile/    # 系统能力配置
```

## 核心功能概览

- **条件监听**: 监听网络、电池、充电、屏幕、存储等系统状态变化，触发延迟任务调度
- **策略管理**: 根据电源模式、内存、热管理、CPU 等系统状态过滤和控制延迟任务执行
- **任务管理**: 延迟任务队列管理、策略协调、进程拉起、数据持久化、超时监控

## 主要方法概览

```cpp
int32_t StartWork(const WorkInfo& workInfo);           // 启动延迟任务
int32_t StopWork(const WorkInfo& workInfo);            // 停止延迟任务
int32_t StopAndCancelWork(const WorkInfo& workInfo);   // 停止并取消延迟任务
int32_t StopAndClearWorks();                           // 停止并清除所有延迟任务
int32_t IsLastWorkTimeout(int32_t workId, bool &result); // 检查上次延迟任务是否超时
int32_t ObtainAllWorks(std::vector<WorkInfo>& workInfos); // 获取所有延迟任务
int32_t GetWorkStatus(int32_t workId, WorkInfo& workInfo); // 获取延迟任务状态
int32_t GetAllRunningWorks(std::vector<WorkInfo>& workInfos); // 获取正在运行的延迟任务
int32_t PauseRunningWorks(int32_t uid);                // 暂停运行的延迟任务
int32_t ResumePausedWorks(int32_t uid);                // 恢复暂停的延迟任务
```

## 整体开发规范

- **日志规范**: 使用 `WS_HILOG*` 宏，`%{public}` 标记公开数据，敏感数据不使用该标记
- **错误码**: `E_PERMISSION_DENIED (201)`、`E_PARAM_ERROR (401)`、`E_CHECK_WORKINFO_FAILED (970000401)` 等
- **条件编译**: `DEVICE_USAGE_STATISTICS_ENABLE`、`DEVICE_STANDBY_ENABLE`、`RESOURCESCHEDULE_BGTASKMGR_ENABLE` 等

## 模块详情链接

- [FRAMEWORKS 模块](./frameworks/FRAMEWORKS.md) - 客户端框架实现
- [INTERFACES 模块](./interfaces/INTERFACES.md) - 公共 API 定义
- [SERVICES 模块](./services/SERVICES.md) - 服务端核心逻辑
- [UTILS 模块](./utils/UTILS.md) - 公共工具类
- [SA_PROFILE 模块](./sa_profile/SA_PROFILE.md) - 系统能力配置