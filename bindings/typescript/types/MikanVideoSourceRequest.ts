// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanVideoSourceIntrinsics, MikanVideoSourceType } from './MikanVideoSourceTypes.js';

export class SetUSBVideoSourceDevice extends MikanRequest {
  video_source_id: number = -1;
  device_path: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'device_path', type: 'string' }
  ];
}

export class SetUSBVideoSourceFormat extends MikanRequest {
  video_source_id: number = -1;
  format: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'format', type: 'string' }
  ];
}

export class SetUSBVideoSourceFrameRate extends MikanRequest {
  video_source_id: number = -1;
  frame_rate: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'frame_rate', type: 'string' }
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

export class GetVideoSourceMode extends MikanRequest {
  video_source_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' }
  ];
}

export class GetVideoSourceIntrinsics extends MikanRequest {
  video_source_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' }
  ];
}

export class SetUSBVideoSourceResolution extends MikanRequest {
  video_source_id: number = -1;
  resolution: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'video_source_id', type: 'int32' },
    { name: 'resolution', type: 'string' }
  ];
}

export class MikanVideoSourceIntrinsicsResponse extends MikanResponse {
  intrinsics: MikanVideoSourceIntrinsics = new MikanVideoSourceIntrinsics();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'intrinsics', type: 'MikanVideoSourceIntrinsics' }
  ];
}

