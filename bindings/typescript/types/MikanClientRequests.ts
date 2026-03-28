// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanClientInfo } from './MikanClientTypes.js';

export class InitClientRequest extends MikanRequest {
  clientInfo: MikanClientInfo = new MikanClientInfo();

  constructor() {
    super();
    this.requestTypeName = 'InitClientRequest';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientInfo', type: 'MikanClientInfo' }
  ];
}

export class DisposeClientRequest extends MikanRequest {
  clientId: string = '';

  constructor() {
    super();
    this.requestTypeName = 'DisposeClientRequest';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientId', type: 'string' }
  ];
}

