// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes';
import { MikanScriptMessageInfo } from './MikanScriptTypes';

export const CLASS_ID_SEND_SCRIPT_MESSAGE = -3006836539234531471n;

export class SendScriptMessage extends MikanRequest {
  message: MikanScriptMessageInfo = new MikanScriptMessageInfo();

  static __serializationMetadata = [
    { name: 'message', type: 'MikanScriptMessageInfo' }
  ];
}

