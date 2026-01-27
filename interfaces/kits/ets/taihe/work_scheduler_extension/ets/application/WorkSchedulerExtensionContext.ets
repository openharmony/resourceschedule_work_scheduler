/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
 
import ExtensionContext from 'application.ExtensionContext';
import Want from '@ohos.app.ability.Want';
import {BusinessError} from '@ohos.base';
 
export default class WorkSchedulerExtensionContext extends ExtensionContext {
    private nativePtr: long = 0;
 
    constructor() {
        super();
        console.log('STS WorkSchedulerExtensionContext constructor');
    }
 
    startServiceExtensionAbility(want: Want): Promise<void> {
        console.log('ani WorkSchedulerExtensionContext startServiceExtensionAbility Promise');
        return taskpool.execute(this.startServiceExtensionAbilityNative, want).then(() => {
            console.log('ani WorkSchedulerExtensionContext startServiceExtensionAbility Promise success');
        }).catch((err: Error) => {
            console.log('ani WorkSchedulerExtensionContext startServiceExtensionAbility Promise catch');
            let error = err as BusinessError;
            throw error;
        });
    }
 
    stopServiceExtensionAbility(want: Want): Promise<void> {
        console.log('ani AccessibilityExtensionContext stopServiceExtensionAbility Promise');
        return taskpool.execute(this.stopServiceExtensionAbilityNative, want).then(() => {
            console.log('ani AccessibilityExtensionContext stopServiceExtensionAbility Promise success');
        }).catch((err: Error) => {
            console.log('ani AccessibilityExtensionContext stopServiceExtensionAbility Promise catch');
            let error = err as BusinessError;
            throw error;
        });
    }
 
    native startServiceExtensionAbilityNative(want: Want): void;
    native stopServiceExtensionAbilityNative(want: Want): void;
}