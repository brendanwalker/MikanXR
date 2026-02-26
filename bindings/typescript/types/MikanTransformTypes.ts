// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';
import { MikanVector3f } from './MikanMathTypes.js';

export const CLASS_ID_MIKAN_TRANSFORM_COMPONENT_VALUES = -1422492652628173064n;

export class MikanTransformComponentValues extends MikanComponentValues {
  relative_scale: MikanVector3f = new MikanVector3f();
  relative_rotation: MikanVector3f = new MikanVector3f();
  relative_position: MikanVector3f = new MikanVector3f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'relative_scale', type: 'MikanVector3f' },
    { name: 'relative_rotation', type: 'MikanVector3f' },
    { name: 'relative_position', type: 'MikanVector3f' }
  ];
}

