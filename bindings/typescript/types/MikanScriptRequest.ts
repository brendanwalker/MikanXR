// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanScriptMessageInfo } from './MikanScriptTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class InvokeComponentScriptTrigger extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = -1;
  trigger_name: string = '';

  constructor() {
    super();
    this.requestTypeName = 'InvokeComponentScriptTrigger';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'trigger_name', type: 'string' }
  ];
}

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

