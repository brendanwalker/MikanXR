// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';
import { MikanQuatf, MikanVector3f } from './MikanMathTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanTransformComponentValues extends MikanComponentValues {
  parent_transform_id: number = -1;
  relative_scale: MikanVector3f = new MikanVector3f();
  relative_quaternion: MikanQuatf = new MikanQuatf();
  relative_position: MikanVector3f = new MikanVector3f();

  static __serializationMetadata: SerializationField[] = [
    { name: 'parent_transform_id', type: 'int32' },
    { name: 'relative_scale', type: 'MikanVector3f' },
    { name: 'relative_quaternion', type: 'MikanQuatf' },
    { name: 'relative_position', type: 'MikanVector3f' }
  ];
}

