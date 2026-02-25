// This file is auto generated. DO NOT EDIT.

import { MikanClientGraphicsApi, MikanColorBufferType, MikanDepthBufferType } from './MikanCoreConstants';

export const CLASS_ID_MIKAN_CLIENT_APIVERSION = 7308303515553926050n;
export const CLASS_ID_MIKAN_RENDER_TARGET_DESCRIPTOR = 8029686814184454925n;

export interface MikanClientAPIVersion {
  version: number;
}

export interface MikanRenderTargetDescriptor {
  color_buffer_type: MikanColorBufferType;
  depth_buffer_type: MikanDepthBufferType;
  width: number;
  height: number;
  graphicsAPI: MikanClientGraphicsApi;
}

