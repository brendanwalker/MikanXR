// This file is auto generated. DO NOT EDIT.

import { MikanRequest } from './MikanAPITypes.js';
import { MikanRenderTargetDescriptor } from './MikanCoreTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class AllocateCameraRenderTargetTextures extends MikanRequest {
  camera_id: number = -1;
  descriptor: MikanRenderTargetDescriptor = new MikanRenderTargetDescriptor();

  constructor() {
    super();
    this.requestTypeName = 'AllocateCameraRenderTargetTextures';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'camera_id', type: 'int32' },
    { name: 'descriptor', type: 'MikanRenderTargetDescriptor' }
  ];
}

export class WriteCameraColorRenderTargetTexture extends MikanRequest {
  camera_id: number = -1;
  api_color_texture_ptr: any = null;

  constructor() {
    super();
    this.requestTypeName = 'WriteCameraColorRenderTargetTexture';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'camera_id', type: 'int32' },
    { name: 'api_color_texture_ptr', type: 'any' }
  ];
}

export class WriteCameraDepthRenderTargetTexture extends MikanRequest {
  camera_id: number = -1;
  api_depth_texture_ptr: any = null;
  z_near: number = 0;
  z_far: number = 0;

  constructor() {
    super();
    this.requestTypeName = 'WriteCameraDepthRenderTargetTexture';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'camera_id', type: 'int32' },
    { name: 'api_depth_texture_ptr', type: 'any' },
    { name: 'z_near', type: 'float' },
    { name: 'z_far', type: 'float' }
  ];
}

export class WriteCameraShadowRenderTargetTexture extends MikanRequest {
  camera_id: number = -1;
  api_shadow_texture_ptr: any = null;

  constructor() {
    super();
    this.requestTypeName = 'WriteCameraShadowRenderTargetTexture';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'camera_id', type: 'int32' },
    { name: 'api_shadow_texture_ptr', type: 'any' }
  ];
}

export class PublishCameraRenderTargetTextures extends MikanRequest {
  camera_id: number = -1;
  frame_index: bigint = 0n;

  constructor() {
    super();
    this.requestTypeName = 'PublishCameraRenderTargetTextures';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'camera_id', type: 'int32' },
    { name: 'frame_index', type: 'int64' }
  ];
}

export class FreeCameraRenderTargetTextures extends MikanRequest {
  camera_id: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'FreeCameraRenderTargetTextures';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'camera_id', type: 'int32' }
  ];
}

