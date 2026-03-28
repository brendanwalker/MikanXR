// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanStencilModelRenderGeometry } from './MikanStencilTypes.js';

export class GetModelStencilRenderGeometry extends MikanRequest {
  stencilId: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'stencilId', type: 'int32' }
  ];
}

export class MikanStencilModelRenderGeometryResponse extends MikanResponse {
  render_geometry: MikanStencilModelRenderGeometry = new MikanStencilModelRenderGeometry();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'render_geometry', type: 'MikanStencilModelRenderGeometry' }
  ];
}

