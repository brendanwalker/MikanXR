// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes';
import { MikanDisconnectCode } from './MikanCoreConstants';
import { MikanClientAPIVersion } from './MikanCoreTypes';

export const CLASS_ID_MIKAN_CONNECTED_EVENT = -8563579496677618876n;
export const CLASS_ID_MIKAN_DISCONNECTED_EVENT = -4899718033844115118n;

export class MikanConnectedEvent extends MikanEvent {
  serverVersion: MikanClientAPIVersion = new MikanClientAPIVersion();
  minClientVersion: MikanClientAPIVersion = new MikanClientAPIVersion();
  isClientCompatible: boolean = false;

  static __serializationMetadata = [
    { name: 'serverVersion', type: 'MikanClientAPIVersion' },
    { name: 'minClientVersion', type: 'MikanClientAPIVersion' },
    { name: 'isClientCompatible', type: 'boolean' }
  ];
}

export class MikanDisconnectedEvent extends MikanEvent {
  code: MikanDisconnectCode = 0;
  reason: string = '';

  static __serializationMetadata = [
    { name: 'code', type: 'enum:MikanDisconnectCode' },
    { name: 'reason', type: 'string' }
  ];
}

