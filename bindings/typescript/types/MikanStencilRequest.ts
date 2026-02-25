// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes';
import { MikanAppStageInfo } from './MikanRemoteControlTypes';
import { MikanStencilModelRenderGeometry } from './MikanStencilTypes';

export const CLASS_ID_GET_MODEL_STENCIL_RENDER_GEOMETRY = 7106057332746101286n;
export const CLASS_ID_MIKAN_APP_STAGE_INFO_RESPONSE = -6661996497938081679n;
export const CLASS_ID_MIKAN_STENCIL_MODEL_RENDER_GEOMETRY_RESPONSE = 6128619420232158675n;

export class GetModelStencilRenderGeometry extends MikanRequest {
  stencilId: number = 0;

  static __serializationMetadata = [
    { name: 'stencilId', type: 'int32' }
  ];
}

export class MikanAppStageInfoResponse extends MikanResponse {
  app_stage_info: MikanAppStageInfo = new MikanAppStageInfo();

  static __serializationMetadata = [
    { name: 'app_stage_info', type: 'MikanAppStageInfo' }
  ];
}

export class MikanStencilModelRenderGeometryResponse extends MikanResponse {
  render_geometry: MikanStencilModelRenderGeometry = new MikanStencilModelRenderGeometry();

  static __serializationMetadata = [
    { name: 'render_geometry', type: 'MikanStencilModelRenderGeometry' }
  ];
}

