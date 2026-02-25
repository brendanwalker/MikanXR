// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes';
import { MikanVideoSourceIntrinsics, MikanVideoSourceType } from './MikanVideoSourceTypes';

export const CLASS_ID_GET_VIDEO_SOURCE_INTRINSICS = 5111634741046016445n;
export const CLASS_ID_GET_VIDEO_SOURCE_MODE = -5470313175782314738n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_INTRINSICS_RESPONSE = 5018187312099351234n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_MODE_RESPONSE = -1059487460754321771n;

export interface GetVideoSourceIntrinsics extends MikanRequest {
  video_source_id: number;
}

export interface GetVideoSourceMode extends MikanRequest {
  video_source_id: number;
}

export interface MikanVideoSourceIntrinsicsResponse extends MikanResponse {
  intrinsics: MikanVideoSourceIntrinsics;
}

export interface MikanVideoSourceModeResponse extends MikanResponse {
  video_source_type: MikanVideoSourceType;
  video_source_api: string;
  device_path: string;
  video_mode_name: string;
  resolution_x: number;
  resolution_y: number;
  frame_rate: number;
}

