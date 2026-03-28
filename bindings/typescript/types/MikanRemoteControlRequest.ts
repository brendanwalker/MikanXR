// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanAppStageInfo } from './MikanRemoteControlTypes.js';

export class GetAppStageInfo extends MikanRequest {

  constructor() {
    super();
    this.requestTypeName = 'GetAppStageInfo';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class PopAppStage extends MikanRequest {

  constructor() {
    super();
    this.requestTypeName = 'PopAppStage';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class MikanRemoteControlCommandResult extends MikanResponse {
  results: string[] = [];

  constructor() {
    super();
    this.responseTypeName = 'MikanRemoteControlCommandResult';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'results', type: 'string', isArray: true }
  ];
}

export class PushAppStage extends MikanRequest {
  app_state_name: string = '';

  constructor() {
    super();
    this.requestTypeName = 'PushAppStage';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'app_state_name', type: 'string' }
  ];
}

export class MikanRemoteControlCommand extends MikanRequest {
  command: string = '';
  parameters: string[] = [];

  constructor() {
    super();
    this.requestTypeName = 'MikanRemoteControlCommand';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'command', type: 'string' },
    { name: 'parameters', type: 'string', isArray: true }
  ];
}

export class MikanAppStageInfoResponse extends MikanResponse {
  app_stage_info: MikanAppStageInfo = new MikanAppStageInfo();

  constructor() {
    super();
    this.responseTypeName = 'MikanAppStageInfoResponse';
  }

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'app_stage_info', type: 'MikanAppStageInfo' }
  ];
}

