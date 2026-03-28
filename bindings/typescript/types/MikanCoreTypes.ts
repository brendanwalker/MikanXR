// This file is auto generated. DO NOT EDIT.

import { MikanClientGraphicsApi, MikanColorBufferType, MikanDepthBufferType } from './MikanCoreConstants.js';

export class MikanClientAPIVersion {
  version: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'version', type: 'int32' }
  ];
}

export class MikanRenderTargetDescriptor {
  color_buffer_type: MikanColorBufferType = MikanColorBufferType.NOCOLOR;
  depth_buffer_type: MikanDepthBufferType = MikanDepthBufferType.NOCOLOR;
  width: number = 0;
  height: number = 0;
  graphicsAPI: MikanClientGraphicsApi = MikanClientGraphicsApi.UNKNOWN;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'color_buffer_type', type: 'enum:MikanColorBufferType' },
    { name: 'depth_buffer_type', type: 'enum:MikanDepthBufferType' },
    { name: 'width', type: 'uint32' },
    { name: 'height', type: 'uint32' },
    { name: 'graphicsAPI', type: 'enum:MikanClientGraphicsApi' }
  ];
}

