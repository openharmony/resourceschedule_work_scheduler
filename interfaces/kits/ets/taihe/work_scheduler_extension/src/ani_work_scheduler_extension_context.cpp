/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#include "ani_work_scheduler_extension_context.h"
#include "ets_extension_context.h"
#include "ani_common_want.h"
#include "ani_utils.h"
#include <ani_signature_builder.h>
#include "work_sched_hilog.h"
#include "work_scheduler_extension.h"
#include "ets_error_utils.h"
 
namespace OHOS {
namespace WorkScheduler {
using namespace arkts::ani_signature;
 
constexpr const char *ANI_EXTENSION_CONTEXT_CLS =
    "application.WorkSchedulerExtensionContext.WorkSchedulerExtensionContext";
constexpr const char *WANT_CLS = "@ohos.app.ability.Want.Want";
 
static void StartServiceExtensionAbility(ani_env *env, ani_object object, ani_object wantObj)
{
    WS_HILOGI("StartServiceExtensionAbility begin");
    WorkSchedulerExtensionContext *extensionContext = ANIUtils::Unwrap<WorkSchedulerExtensionContext>(env, object);
    if (extensionContext == nullptr) {
        WS_HILOGE("extensionContext is nullptr");
        AbilityRuntime::EtsErrorUtil::ThrowTooFewParametersError(env);
        return;
    }
    arkts::ani_signature::Type wantClass = Builder::BuildClass(WANT_CLS);
    ani_class cls;
    if (env->FindClass(wantClass.Descriptor().c_str(), &cls) != ANI_OK) {
        WS_HILOGE("find class Want failed");
        AbilityRuntime::EtsErrorUtil::ThrowTooFewParametersError(env);
        return;
    }
    ani_boolean isWant;
    if (ANI_OK != env->Object_InstanceOf(static_cast<ani_object>(object), static_cast<ani_type>(cls), &isWant)) {
        WS_HILOGE("call object instance of Want failed");
        AbilityRuntime::EtsErrorUtil::ThrowTooFewParametersError(env);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        WS_HILOGE("call UnWrapWant failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
        return;
    }
    auto innerErrorCode = extensionContext->StartServiceExtensionAbility(want);
    if (innerErrorCode != ERR_OK) {
        WS_HILOGE("StartServiceExtensionAbility error");
        AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, innerErrorCode);
        return;
    }
    WS_HILOGI("StartServiceExtensionAbility end");
}
 
static void StopServiceExtensionAbility(ani_env *env, ani_object object, ani_object wantObj)
{
    WS_HILOGI("StopServiceExtensionAbility begin");
    WorkSchedulerExtensionContext *extensionContext = ANIUtils::Unwrap<WorkSchedulerExtensionContext>(env, object);
    if (extensionContext == nullptr) {
        WS_HILOGE("extensionContext is nullptr");
        AbilityRuntime::EtsErrorUtil::ThrowTooFewParametersError(env);
        return;
    }
    arkts::ani_signature::Type wantClass = Builder::BuildClass(WANT_CLS);
    ani_class cls;
    if (env->FindClass(wantClass.Descriptor().c_str(), &cls) != ANI_OK) {
        WS_HILOGE("find class Want failed");
        AbilityRuntime::EtsErrorUtil::ThrowTooFewParametersError(env);
        return;
    }
    ani_boolean isWant;
    if (ANI_OK != env->Object_InstanceOf(static_cast<ani_object>(object), static_cast<ani_type>(cls), &isWant)) {
        WS_HILOGE("call object instance of Want failed");
        AbilityRuntime::EtsErrorUtil::ThrowTooFewParametersError(env);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        WS_HILOGE("call UnWrapWant failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
        return;
    }
    auto innerErrorCode = extensionContext->StopServiceExtensionAbility(want);
    if (innerErrorCode != ERR_OK) {
        WS_HILOGE("StopServiceExtensionAbility error");
        AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, innerErrorCode);
        return;
    }
    WS_HILOGI("StopServiceExtensionAbility end");
}
 
ani_object CreateAniWorkSchedulerExtensionContext(ani_env *env, std::shared_ptr<WorkSchedulerExtensionContext> context,
    const std::shared_ptr<AbilityRuntime::OHOSApplication> &application)
{
    WS_HILOGD("CreateAniWorkSchedulerExtensionContext begin");
    ani_class cls;
    arkts::ani_signature::Type className = arkts::ani_signature::Builder::BuildClass(ANI_EXTENSION_CONTEXT_CLS);
    if (ANI_OK != env->FindClass(className.Descriptor().c_str(), &cls)) {
        WS_HILOGE("Ani FindClass err: extention context");
        return nullptr;
    }
    std::array methods = {
        ani_native_function {"startServiceExtensionAbilityNative",
            nullptr, reinterpret_cast<void *>(StartServiceExtensionAbility)},
        ani_native_function {"stopServiceExtensionAbilityNative",
            nullptr, reinterpret_cast<void *>(StopServiceExtensionAbility)},
    };
    WS_HILOGI("CreateAniWorkSchedulerExtensionContext bind context methods");
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        WS_HILOGE("Cannot bind native methods to extension context");
        return nullptr;
    }
    ani_object contextObj = CreateObject(env, cls);
    if (ANI_OK != ANIUtils::Wrap(env, contextObj, context.get())) {
        WS_HILOGE("Cannot wrap native object");
        return nullptr;
    }
    OHOS::AbilityRuntime::CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    WS_HILOGI("CreateAniWorkSchedulerExtensionContext end");
    return contextObj;
}
 
ani_object CreateObject(ani_env *env, ani_class cls, ...)
{
    ani_object nullobj {};
 
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        WS_HILOGE("[ANI] Not found <ctor> for class");
        return nullobj;
    }
 
    ani_object obj;
    va_list args;
    va_start(args, cls);
    ani_status status = env->Object_New_V(cls, ctor, &obj, args);
    va_end(args);
    if (ANI_OK != status) {
        WS_HILOGE("[ANI] Failed to Object_New for class 1");
        return nullobj;
    }
    return obj;
}
} // namespace WorkScheduler
} // namespace OHOS