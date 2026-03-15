// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanVideoSourceIntrinsics, MikanVideoSourceType } from './MikanVideoSourceTypes.js';

export const CLASS_ID_GET_VIDEO_SOURCE_INTRINSICS = 5111634741046016445n;
export const CLASS_ID_GET_VIDEO_SOURCE_MODE = -5470313175782314738n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_INTRINSICS_RESPONSE = 5018187312099351234n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_MODE_RESPONSE = -1059487460754321771n;
export const CLASS_ID_SET_USBVIDEO_SOURCE_DEVICE = -5535407022205852385n;
export const CLASS_ID_SET_USBVIDEO_SOURCE_FORMAT = -9142657520890963442n;
export const CLASS_ID_SET_USBVIDEO_SOURCE_FRAME_RATE = -470593085450153328n;
export const CLASS_ID_SET_USBVIDEO_SOURCE_RESOLUTION = -8879922715685289037n;

export class GetVideoSourceIntrinsics extends MikanRequest {
  video_source_id: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' }
  ];
}

export class GetVideoSourceMode extends MikanRequest {
  video_source_id: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' }
  ];
}

export class MikanVideoSourceIntrinsicsResponse extends MikanResponse {
  intrinsics: MikanVideoSourceIntrinsics = new MikanVideoSourceIntrinsics();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'intrinsics', type: 'MikanVideoSourceIntrinsics' }
  ];
}

export class MikanVideoSourceModeResponse extends MikanResponse {
  video_source_type: MikanVideoSourceType = MikanVideoSourceType.MONO;
  video_source_api: string = '';
  device_path: string = '';
  video_mode_name: string = '';
  resolution_x: number = 0;
  resolution_y: number = 0;
  frame_rate: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_type', type: 'enum:MikanVideoSourceType' },
    { name: 'video_source_api', type: 'string' },
    { name: 'device_path', type: 'string' },
    { name: 'video_mode_name', type: 'string' },
    { name: 'resolution_x', type: 'int32' },
    { name: 'resolution_y', type: 'int32' },
    { name: 'frame_rate', type: 'float' }
  ];
}

export class SetUSBVideoSourceDevice extends MikanRequest {
  video_source_id: number = 0;
  device_path: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'device_path', type: 'string' }
  ];
}

export class SetUSBVideoSourceFormat extends MikanRequest {
  video_source_id: number = 0;
  format: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'format', type: 'string' }
  ];
}

export class SetUSBVideoSourceFrameRate extends MikanRequest {
  video_source_id: number = 0;
  frame_rate: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'frame_rate', type: 'string' }
  ];
}

export class SetUSBVideoSourceResolution extends MikanRequest {
  video_source_id: number = 0;
  resolution: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'resolution', type: 'string' }
  ];
}

