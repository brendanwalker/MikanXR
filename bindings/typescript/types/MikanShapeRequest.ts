// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanStencilModelRenderGeometry } from './MikanStencilTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class GetModelShapeRenderGeometry extends MikanRequest {
  shapeId: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'GetModelShapeRenderGeometry';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'shapeId', type: 'int32' }
  ];
}

export class MikanShapeModelRenderGeometryResponse extends MikanResponse {
  render_geometry: MikanStencilModelRenderGeometry = new MikanStencilModelRenderGeometry();

  constructor() {
    super();
    this.responseTypeName = 'MikanShapeModelRenderGeometryResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'render_geometry', type: 'MikanStencilModelRenderGeometry' }
  ];
}

