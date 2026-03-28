// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';
import { MikanVector3f } from './MikanMathTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanTransformComponentValues extends MikanComponentValues {
  parent_transform_id: number = -1;
  relative_scale: MikanVector3f = new MikanVector3f();
  relative_rotation: MikanVector3f = new MikanVector3f();
  relative_position: MikanVector3f = new MikanVector3f();

  static __serializationMetadata: SerializationField[] = [
    { name: 'parent_transform_id', type: 'int32' },
    { name: 'relative_scale', type: 'MikanVector3f' },
    { name: 'relative_rotation', type: 'MikanVector3f' },
    { name: 'relative_position', type: 'MikanVector3f' }
  ];
}

