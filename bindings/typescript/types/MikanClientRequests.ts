// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes';
import { MikanClientInfo } from './MikanClientTypes';

export const CLASS_ID_DISPOSE_CLIENT_REQUEST = -671320724823045972n;
export const CLASS_ID_INIT_CLIENT_REQUEST = 7270577563897270843n;

export class DisposeClientRequest extends MikanRequest {
  clientId: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientId', type: 'string' }
  ];
}

export class InitClientRequest extends MikanRequest {
  clientInfo: MikanClientInfo = new MikanClientInfo();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'clientInfo', type: 'MikanClientInfo' }
  ];
}

