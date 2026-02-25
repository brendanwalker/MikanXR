// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes';
import { MikanRenderTargetDescriptor } from './MikanCoreTypes';

export const CLASS_ID_ALLOCATE_CAMERA_RENDER_TARGET_TEXTURES = -6829988777871465530n;
export const CLASS_ID_FREE_CAMERA_RENDER_TARGET_TEXTURES = 7456227028164264559n;
export const CLASS_ID_PUBLISH_CAMERA_RENDER_TARGET_TEXTURES = 9149889041987241612n;
export const CLASS_ID_WRITE_CAMERA_COLOR_RENDER_TARGET_TEXTURE = 6474596006422811888n;
export const CLASS_ID_WRITE_CAMERA_DEPTH_RENDER_TARGET_TEXTURE = -1802673963658192754n;

export interface AllocateCameraRenderTargetTextures extends MikanRequest {
  camera_id: number;
  descriptor: MikanRenderTargetDescriptor;
}

export interface FreeCameraRenderTargetTextures extends MikanRequest {
  camera_id: number;
}

export interface PublishCameraRenderTargetTextures extends MikanRequest {
  camera_id: number;
  frame_index: bigint;
}

export interface WriteCameraColorRenderTargetTexture extends MikanRequest {
  camera_id: number;
  api_color_texture_ptr: any;
}

export interface WriteCameraDepthRenderTargetTexture extends MikanRequest {
  camera_id: number;
  api_depth_texture_ptr: any;
  z_near: number;
  z_far: number;
}

