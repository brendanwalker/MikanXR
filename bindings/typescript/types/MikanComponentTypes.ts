// This file is auto generated. DO NOT EDIT.

import { PolymorphicStruct } from './../PolymorphicObject.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanComponentValues extends PolymorphicStruct {
  component_id: number = -1;
  component_name: string = '';

  static __serializationMetadata: SerializationField[] = [
    { name: 'component_id', type: 'int32' },
    { name: 'component_name', type: 'string' }
  ];
}

