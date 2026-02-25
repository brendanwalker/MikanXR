// This file is auto generated. DO NOT EDIT.

import { MikanVariant, MikanVariantType } from './MikanVariantTypes';

export const CLASS_ID_MIKAN_PROPERTY_DESCRIPTOR = 6424203277621511357n;
export const CLASS_ID_MIKAN_PROPERTY_VALUE = 6952539342367057093n;
export const CLASS_ID_MIKAN_SYSTEM_VALUES = 6499163311643253046n;

export class MikanPropertyDescriptor {
  ownerSystemClass: string = '';
  ownerComponentClass: string = '';
  fieldName: string = '';
  fieldType: MikanVariantType = 0;
  isReadOnly: boolean = false;

  static __serializationMetadata = [
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

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'ownerComponentClass', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'fieldName', type: 'string' },
    { name: 'fieldValue', type: 'MikanVariant' }
  ];
}

export class MikanSystemValues extends PolymorphicStruct {

  static __serializationMetadata = [
  ];
}

