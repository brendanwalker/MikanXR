// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanClientInfo } from './MikanClientTypes.js';

export class InitClientRequest extends MikanRequest {
  clientInfo: MikanClientInfo = new MikanClientInfo();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientInfo', type: 'MikanClientInfo' }
  ];
}

export class DisposeClientRequest extends MikanRequest {
  clientId: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientId', type: 'string' }
  ];
}

