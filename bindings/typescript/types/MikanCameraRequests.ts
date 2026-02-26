// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanRenderTargetDescriptor } from './MikanCoreTypes.js';

export const CLASS_ID_ALLOCATE_CAMERA_RENDER_TARGET_TEXTURES = -6829988777871465530n;
export const CLASS_ID_FREE_CAMERA_RENDER_TARGET_TEXTURES = 7456227028164264559n;
export const CLASS_ID_PUBLISH_CAMERA_RENDER_TARGET_TEXTURES = 9149889041987241612n;
export const CLASS_ID_WRITE_CAMERA_COLOR_RENDER_TARGET_TEXTURE = 6474596006422811888n;
export const CLASS_ID_WRITE_CAMERA_DEPTH_RENDER_TARGET_TEXTURE = -1802673963658192754n;

export class AllocateCameraRenderTargetTextures extends MikanRequest {
  camera_id: number = 0;
  descriptor: MikanRenderTargetDescriptor = new MikanRenderTargetDescriptor();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'camera_id', type: 'int32' },
    { name: 'descriptor', type: 'MikanRenderTargetDescriptor' }
  ];
}

export class FreeCameraRenderTargetTextures extends MikanRequest {
  camera_id: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'camera_id', type: 'int32' }
  ];
}

export class PublishCameraRenderTargetTextures extends MikanRequest {
  camera_id: number = 0;
  frame_index: bigint = 0n;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'camera_id', type: 'int32' },
    { name: 'frame_index', type: 'int64' }
  ];
}

export class WriteCameraColorRenderTargetTexture extends MikanRequest {
  camera_id: number = 0;
  api_color_texture_ptr: any = null;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'camera_id', type: 'int32' },
    { name: 'api_color_texture_ptr', type: 'any' }
  ];
}

export class WriteCameraDepthRenderTargetTexture extends MikanRequest {
  camera_id: number = 0;
  api_depth_texture_ptr: any = null;
  z_near: number = 0;
  z_far: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'camera_id', type: 'int32' },
    { name: 'api_depth_texture_ptr', type: 'any' },
    { name: 'z_near', type: 'float' },
    { name: 'z_far', type: 'float' }
  ];
}

