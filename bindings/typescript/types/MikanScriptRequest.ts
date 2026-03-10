// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanScriptMessageInfo } from './MikanScriptTypes.js';

export const CLASS_ID_INVOKE_COMPONENT_SCRIPT_TRIGGER = -8350650813685957365n;
export const CLASS_ID_SEND_SCRIPT_MESSAGE = -3006836539234531471n;

export class InvokeComponentScriptTrigger extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = 0;
  trigger_name: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'trigger_name', type: 'string' }
  ];
}

export class SendScriptMessage extends MikanRequest {
  message: MikanScriptMessageInfo = new MikanScriptMessageInfo();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'message', type: 'MikanScriptMessageInfo' }
  ];
}

