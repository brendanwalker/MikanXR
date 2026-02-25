// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes';

export const CLASS_ID_MIKAN_APP_STAGE_CHANGED_EVENT = 373547867812003606n;
export const CLASS_ID_MIKAN_REMOTE_CONTROL_EVENT = -1767585761143530576n;

export interface MikanAppStageChangedEvent extends MikanEvent {
  new_app_state_name: string;
  old_app_state_name: string;
}

export interface MikanRemoteControlEvent extends MikanEvent {
  remoteControlEvent: string;
  parameters: string[];
}

