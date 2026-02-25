// This file is auto generated. DO NOT EDIT.

import { MikanVector2f, MikanVector3f } from './MikanMathTypes';
import { MikanTransformComponentValues } from './MikanTransformTypes';

export enum MikanStencilCullMode {
  NONE = 0,
  Z_Axis = 1,
  Y_Axis = 2,
  X_Axis = 3
}

export const CLASS_ID_MIKAN_BOX_STENCIL_COMPONENT_VALUES = -6788717745397413241n;
export const CLASS_ID_MIKAN_MODEL_STENCIL_COMPONENT_VALUES = 5055554930502929791n;
export const CLASS_ID_MIKAN_QUAD_STENCIL_COMPONENT_VALUES = -9026237790691884165n;
export const CLASS_ID_MIKAN_STENCIL_COMPONENT_VALUES = -4451290801219034056n;
export const CLASS_ID_MIKAN_STENCIL_MODEL_RENDER_GEOMETRY = 6822885306325183796n;
export const CLASS_ID_MIKAN_TRIAGULATED_MESH = -1925804809077911022n;

export interface MikanBoxStencilComponentValues extends MikanStencilComponentValues {
  box_x_size: number;
  box_y_size: number;
  box_z_size: number;
}

export interface MikanModelStencilComponentValues extends MikanStencilComponentValues {
  model_path: string;
}

export interface MikanQuadStencilComponentValues extends MikanStencilComponentValues {
  quad_width: number;
  quad_height: number;
  is_double_sided: boolean;
}

export interface MikanStencilComponentValues extends MikanTransformComponentValues {
  parent_anchor_id: number;
  is_disabled: boolean;
  cull_mode: MikanStencilCullMode;
}

export interface MikanStencilModelRenderGeometry {
  meshes: MikanTriagulatedMesh[];
}

export interface MikanTriagulatedMesh {
  vertices: MikanVector3f[];
  normals: MikanVector3f[];
  texels: MikanVector2f[];
  indices: number[];
}

