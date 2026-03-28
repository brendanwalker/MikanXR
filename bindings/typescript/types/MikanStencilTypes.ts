// This file is auto generated. DO NOT EDIT.

import { MikanVector2f, MikanVector3f } from './MikanMathTypes.js';
import { MikanSystemValues } from './MikanPropertyTypes.js';
import { MikanTransformComponentValues } from './MikanTransformTypes.js';

export enum MikanStencilCullMode {
  NONE = 0,
  Z_Axis = 1,
  Y_Axis = 2,
  X_Axis = 3
}

export class MikanStencilComponentValues extends MikanTransformComponentValues {
  is_disabled: boolean = false;
  cull_mode: MikanStencilCullMode = MikanStencilCullMode.NONE;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'is_disabled', type: 'boolean' },
    { name: 'cull_mode', type: 'enum:MikanStencilCullMode' }
  ];
}

export class MikanModelStencilComponentValues extends MikanStencilComponentValues {
  model_path: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'model_path', type: 'string' }
  ];
}

export class MikanQuadStencilSystemValues extends MikanSystemValues {
  render_stencils: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'render_stencils', type: 'boolean' }
  ];
}

export class MikanBoxStencilSystemValues extends MikanSystemValues {
  render_stencils: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'render_stencils', type: 'boolean' }
  ];
}

export class MikanModelStencilSystemValues extends MikanSystemValues {
  render_stencils: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'render_stencils', type: 'boolean' }
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

export class MikanStencilModelRenderGeometry {
  meshes: MikanTriagulatedMesh[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'meshes', type: 'MikanTriagulatedMesh', isArray: true }
  ];
}

