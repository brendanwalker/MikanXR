// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes';

export const CLASS_ID_GET_APP_STAGE_INFO = -1337747226464149062n;
export const CLASS_ID_MIKAN_REMOTE_CONTROL_COMMAND = 4595909365701644961n;
export const CLASS_ID_MIKAN_REMOTE_CONTROL_COMMAND_RESULT = -9034381214596710800n;
export const CLASS_ID_POP_APP_STAGE = -589658948136412267n;
export const CLASS_ID_PUSH_APP_STAGE = -7872424436528764660n;

export interface GetAppStageInfo extends MikanRequest {
}

export interface MikanRemoteControlCommand extends MikanRequest {
  command: string;
  parameters: string[];
}

export interface MikanRemoteControlCommandResult extends MikanResponse {
  results: string[];
}

export interface PopAppStage extends MikanRequest {
}

export interface PushAppStage extends MikanRequest {
  app_state_name: string;
}

