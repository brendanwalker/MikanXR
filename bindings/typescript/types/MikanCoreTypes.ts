// This file is auto generated. DO NOT EDIT.

import { MikanClientGraphicsApi, MikanColorBufferType, MikanDepthBufferType } from './MikanCoreConstants';

export const CLASS_ID_MIKAN_CLIENT_APIVERSION = 7308303515553926050n;
export const CLASS_ID_MIKAN_RENDER_TARGET_DESCRIPTOR = 8029686814184454925n;

export class MikanClientAPIVersion {
  version: number = 0;

  static __serializationMetadata = [
    { name: 'version', type: 'int32' }
  ];
}

export class MikanRenderTargetDescriptor {
  color_buffer_type: MikanColorBufferType = 0;
  depth_buffer_type: MikanDepthBufferType = 0;
  width: number = 0;
  height: number = 0;
  graphicsAPI: MikanClientGraphicsApi = 0;

  static __serializationMetadata = [
    { name: 'color_buffer_type', type: 'enum:MikanColorBufferType' },
    { name: 'depth_buffer_type', type: 'enum:MikanDepthBufferType' },
    { name: 'width', type: 'uint32' },
    { name: 'height', type: 'uint32' },
    { name: 'graphicsAPI', type: 'enum:MikanClientGraphicsApi' }
  ];
}

