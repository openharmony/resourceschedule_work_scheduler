## SA_PROFILE 模块

### 功能概述
系统能力配置文件，定义 WorkScheduler 服务启动参数。

### 配置文件说明

**服务配置 (1904.json)**
- **服务 ID**: 1904 (WORK_SCHEDULE_SERVICE_ID)
- **进程名**: resource_schedule_service
- **动态库路径**: libworkschedservice.z.so
- **启动类型**: run-on-create = true，服务随进程启动自动创建
- **分布式支持**: distributed = false
- **Dump 级别**: dump_level = 1

### 配置规范

**修改配置**
1. 编辑 `1904.json` 调整启动参数
2. 修改 `run-on-create` 控制按需启动或自动启动
3. 调整 `dump_level` 设置调试信息输出级别

**扩展配置**
- 新增系统能力需在 `systemability` 数组中添加配置项
- 每个配置项需包含 `name` (服务ID) 和 `libpath` (动态库路径)
- 配置修改后需重新编译部件生效