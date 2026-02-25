// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes';
import { MikanClientInfo } from './MikanClientTypes';

export const CLASS_ID_DISPOSE_CLIENT_REQUEST = -671320724823045972n;
export const CLASS_ID_INIT_CLIENT_REQUEST = 7270577563897270843n;

export interface DisposeClientRequest extends MikanRequest {
  clientId: string;
}

export interface InitClientRequest extends MikanRequest {
  clientInfo: MikanClientInfo;
}

