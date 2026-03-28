// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes.js';

export class MikanAppStageChangedEvent extends MikanEvent {
  new_app_state_name: string = '';
  old_app_state_name: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'new_app_state_name', type: 'string' },
    { name: 'old_app_state_name', type: 'string' }
  ];
}

export class MikanRemoteControlEvent extends MikanEvent {
  remoteControlEvent: string = '';
  parameters: string[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'remoteControlEvent', type: 'string' },
    { name: 'parameters', type: 'string', isArray: true }
  ];
}

