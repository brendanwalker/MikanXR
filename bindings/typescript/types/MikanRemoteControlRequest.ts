// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes';

export const CLASS_ID_GET_APP_STAGE_INFO = -1337747226464149062n;
export const CLASS_ID_MIKAN_REMOTE_CONTROL_COMMAND = 4595909365701644961n;
export const CLASS_ID_MIKAN_REMOTE_CONTROL_COMMAND_RESULT = -9034381214596710800n;
export const CLASS_ID_POP_APP_STAGE = -589658948136412267n;
export const CLASS_ID_PUSH_APP_STAGE = -7872424436528764660n;

export class GetAppStageInfo extends MikanRequest {

  static __serializationMetadata = [
  ];
}

export class MikanRemoteControlCommand extends MikanRequest {
  command: string = '';
  parameters: string[] = [];

  static __serializationMetadata = [
    { name: 'command', type: 'string' },
    { name: 'parameters', type: 'string', isArray: true }
  ];
}

export class MikanRemoteControlCommandResult extends MikanResponse {
  results: string[] = [];

  static __serializationMetadata = [
    { name: 'results', type: 'string', isArray: true }
  ];
}

export class PopAppStage extends MikanRequest {

  static __serializationMetadata = [
  ];
}

export class PushAppStage extends MikanRequest {
  app_state_name: string = '';

  static __serializationMetadata = [
    { name: 'app_state_name', type: 'string' }
  ];
}

