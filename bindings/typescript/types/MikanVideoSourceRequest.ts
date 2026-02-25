// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes';
import { MikanVideoSourceIntrinsics, MikanVideoSourceType } from './MikanVideoSourceTypes';

export const CLASS_ID_GET_VIDEO_SOURCE_INTRINSICS = 5111634741046016445n;
export const CLASS_ID_GET_VIDEO_SOURCE_MODE = -5470313175782314738n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_INTRINSICS_RESPONSE = 5018187312099351234n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_MODE_RESPONSE = -1059487460754321771n;

export class GetVideoSourceIntrinsics extends MikanRequest {
  video_source_id: number = 0;

  static __serializationMetadata = [
    { name: 'video_source_id', type: 'int32' }
  ];
}

export class GetVideoSourceMode extends MikanRequest {
  video_source_id: number = 0;

  static __serializationMetadata = [
    { name: 'video_source_id', type: 'int32' }
  ];
}

export class MikanVideoSourceIntrinsicsResponse extends MikanResponse {
  intrinsics: MikanVideoSourceIntrinsics = new MikanVideoSourceIntrinsics();

  static __serializationMetadata = [
    { name: 'intrinsics', type: 'MikanVideoSourceIntrinsics' }
  ];
}

export class MikanVideoSourceModeResponse extends MikanResponse {
  video_source_type: MikanVideoSourceType = 0;
  video_source_api: string = '';
  device_path: string = '';
  video_mode_name: string = '';
  resolution_x: number = 0;
  resolution_y: number = 0;
  frame_rate: number = 0;

  static __serializationMetadata = [
    { name: 'video_source_type', type: 'enum:MikanVideoSourceType' },
    { name: 'video_source_api', type: 'string' },
    { name: 'device_path', type: 'string' },
    { name: 'video_mode_name', type: 'string' },
    { name: 'resolution_x', type: 'int32' },
    { name: 'resolution_y', type: 'int32' },
    { name: 'frame_rate', type: 'float' }
  ];
}

