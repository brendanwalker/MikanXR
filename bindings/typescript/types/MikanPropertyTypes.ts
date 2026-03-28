// This file is auto generated. DO NOT EDIT.

import { PolymorphicStruct } from './../PolymorphicObject.js';
import { MikanVariant, MikanVariantType } from './MikanVariantTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanSystemValues extends PolymorphicStruct {

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanPropertyDescriptor {
  ownerSystemClass: string = '';
  ownerComponentClass: string = '';
  fieldName: string = '';
  fieldType: MikanVariantType = MikanVariantType.INVALID_TYPE;
  isReadOnly: boolean = false;

  static __serializationMetadata: SerializationField[] = [
    { name: 'ownerSystemClass', type: 'string' },
    { name: 'ownerComponentClass', type: 'string' },
    { name: 'fieldName', type: 'string' },
    { name: 'fieldType', type: 'enum:MikanVariantType' },
    { name: 'isReadOnly', type: 'boolean' }
  ];
}

export class MikanPropertyValue {
  ownerSystem: string = '';
  ownerComponentClass: string = '';
  componentId: number = 0;
  fieldName: string = '';
  fieldValue: MikanVariant = new MikanVariant();

  static __serializationMetadata: SerializationField[] = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'ownerComponentClass', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'fieldName', type: 'string' },
    { name: 'fieldValue', type: 'MikanVariant' }
  ];
}

