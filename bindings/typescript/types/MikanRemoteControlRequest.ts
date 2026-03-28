// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanAppStageInfo } from './MikanRemoteControlTypes.js';

export class GetAppStageInfo extends MikanRequest {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class PopAppStage extends MikanRequest {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class MikanRemoteControlCommandResult extends MikanResponse {
  results: string[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'results', type: 'string', isArray: true }
  ];
}

export class PushAppStage extends MikanRequest {
  app_state_name: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'app_state_name', type: 'string' }
  ];
}

export class MikanRemoteControlCommand extends MikanRequest {
  command: string = '';
  parameters: string[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'command', type: 'string' },
    { name: 'parameters', type: 'string', isArray: true }
  ];
}

export class MikanAppStageInfoResponse extends MikanResponse {
  app_stage_info: MikanAppStageInfo = new MikanAppStageInfo();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'app_stage_info', type: 'MikanAppStageInfo' }
  ];
}

