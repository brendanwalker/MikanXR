// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanStencilModelRenderGeometry } from './MikanStencilTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class GetModelStencilRenderGeometry extends MikanRequest {
  stencilId: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'GetModelStencilRenderGeometry';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'stencilId', type: 'int32' }
  ];
}

export class MikanStencilModelRenderGeometryResponse extends MikanResponse {
  render_geometry: MikanStencilModelRenderGeometry = new MikanStencilModelRenderGeometry();

  constructor() {
    super();
    this.responseTypeName = 'MikanStencilModelRenderGeometryResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'render_geometry', type: 'MikanStencilModelRenderGeometry' }
  ];
}

