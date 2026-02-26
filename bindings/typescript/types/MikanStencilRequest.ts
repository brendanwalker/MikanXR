// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanStencilModelRenderGeometry } from './MikanStencilTypes.js';

export const CLASS_ID_GET_MODEL_STENCIL_RENDER_GEOMETRY = 7106057332746101286n;
export const CLASS_ID_MIKAN_STENCIL_MODEL_RENDER_GEOMETRY_RESPONSE = 6128619420232158675n;

export class GetModelStencilRenderGeometry extends MikanRequest {
  stencilId: number = 0;

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

