// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanScriptMessageInfo } from './MikanScriptTypes.js';

export const CLASS_ID_SEND_SCRIPT_MESSAGE = -3006836539234531471n;

export class SendScriptMessage extends MikanRequest {
  message: MikanScriptMessageInfo = new MikanScriptMessageInfo();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'message', type: 'MikanScriptMessageInfo' }
  ];
}

