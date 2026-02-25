// This file is auto generated. DO NOT EDIT.

import { MikanVariant, MikanVariantType } from './MikanVariantTypes';

export const CLASS_ID_MIKAN_PROPERTY_DESCRIPTOR = 6424203277621511357n;
export const CLASS_ID_MIKAN_PROPERTY_VALUE = 6952539342367057093n;
export const CLASS_ID_MIKAN_SYSTEM_VALUES = 6499163311643253046n;

export interface MikanPropertyDescriptor {
  ownerSystemClass: string;
  ownerComponentClass: string;
  fieldName: string;
  fieldType: MikanVariantType;
  isReadOnly: boolean;
}

export interface MikanPropertyValue {
  ownerSystem: string;
  ownerComponentClass: string;
  componentId: number;
  fieldName: string;
  fieldValue: MikanVariant;
}

export interface MikanSystemValues extends PolymorphicStruct {
}

