// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes.js';
import { MikanDisconnectCode } from './MikanCoreConstants.js';
import { MikanClientAPIVersion } from './MikanCoreTypes.js';

export class MikanConnectedEvent extends MikanEvent {
  serverVersion: MikanClientAPIVersion = new MikanClientAPIVersion();
  minClientVersion: MikanClientAPIVersion = new MikanClientAPIVersion();
  isClientCompatible: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'serverVersion', type: 'MikanClientAPIVersion' },
    { name: 'minClientVersion', type: 'MikanClientAPIVersion' },
    { name: 'isClientCompatible', type: 'boolean' }
  ];
}

export class MikanDisconnectedEvent extends MikanEvent {
  code: MikanDisconnectCode = MikanDisconnectCode.Normal;
  reason: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'code', type: 'enum:MikanDisconnectCode' },
    { name: 'reason', type: 'string' }
  ];
}

