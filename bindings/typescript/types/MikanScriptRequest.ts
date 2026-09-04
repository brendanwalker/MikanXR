// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanScriptMessageInfo } from './MikanScriptTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class SendScriptMessage extends MikanRequest {
  message: MikanScriptMessageInfo = new MikanScriptMessageInfo();

  constructor() {
    super();
    this.requestTypeName = 'SendScriptMessage';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'message', type: 'MikanScriptMessageInfo' }
  ];
}

export class InvokeScriptTrigger extends MikanRequest {
  trigger_name: string = '';

  constructor() {
    super();
    this.requestTypeName = 'InvokeScriptTrigger';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'trigger_name', type: 'string' }
  ];
}

