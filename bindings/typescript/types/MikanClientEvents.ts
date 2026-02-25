// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes';
import { MikanDisconnectCode } from './MikanCoreConstants';
import { MikanClientAPIVersion } from './MikanCoreTypes';

export const CLASS_ID_MIKAN_CONNECTED_EVENT = -8563579496677618876n;
export const CLASS_ID_MIKAN_DISCONNECTED_EVENT = -4899718033844115118n;

export interface MikanConnectedEvent extends MikanEvent {
  serverVersion: MikanClientAPIVersion;
  minClientVersion: MikanClientAPIVersion;
  isClientCompatible: boolean;
}

export interface MikanDisconnectedEvent extends MikanEvent {
  code: MikanDisconnectCode;
  reason: string;
}

