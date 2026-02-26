// This file is auto generated. DO NOT EDIT.

import { MikanVector2f, MikanVector3f } from './MikanMathTypes.js';
import { MikanTransformComponentValues } from './MikanTransformTypes.js';

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

export class MikanStencilComponentValues extends MikanTransformComponentValues {
  parent_anchor_id: number = 0;
  is_disabled: boolean = false;
  cull_mode: MikanStencilCullMode = MikanStencilCullMode.NONE;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'parent_anchor_id', type: 'int32' },
    { name: 'is_disabled', type: 'boolean' },
    { name: 'cull_mode', type: 'enum:MikanStencilCullMode' }
  ];
}

export class MikanBoxStencilComponentValues extends MikanStencilComponentValues {
  box_x_size: number = 0;
  box_y_size: number = 0;
  box_z_size: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'box_x_size', type: 'float' },
    { name: 'box_y_size', type: 'float' },
    { name: 'box_z_size', type: 'float' }
  ];
}

export class MikanModelStencilComponentValues extends MikanStencilComponentValues {
  model_path: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'model_path', type: 'string' }
  ];
}

export class MikanQuadStencilComponentValues extends MikanStencilComponentValues {
  quad_width: number = 0;
  quad_height: number = 0;
  is_double_sided: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'quad_width', type: 'float' },
    { name: 'quad_height', type: 'float' },
    { name: 'is_double_sided', type: 'boolean' }
  ];
}

export class MikanStencilModelRenderGeometry {
  meshes: MikanTriagulatedMesh[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'meshes', type: 'MikanTriagulatedMesh', isArray: true }
  ];
}

export class MikanTriagulatedMesh {
  vertices: MikanVector3f[] = [];
  normals: MikanVector3f[] = [];
  texels: MikanVector2f[] = [];
  indices: number[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'vertices', type: 'MikanVector3f', isArray: true },
    { name: 'normals', type: 'MikanVector3f', isArray: true },
    { name: 'texels', type: 'MikanVector2f', isArray: true },
    { name: 'indices', type: 'int32', isArray: true }
  ];
}

