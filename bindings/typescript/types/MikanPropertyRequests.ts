// This file is auto generated. DO NOT EDIT.

import { PolymorphicObject, PolymorphicStruct } from './../PolymorphicObject';
import { MikanRequest, MikanResponse } from './MikanAPITypes';
import { MikanPropertyDescriptor, MikanPropertyValue } from './MikanPropertyTypes';
import { MikanVariant } from './MikanVariantTypes';

export enum MikanPropertyNotifyMode {
  NONE = 0,
  NAME = 1,
  NAME_AND_VALUE = 2
}

export const CLASS_ID_COMPONENT_GET_VALUES_REQUEST = -4733146441529199375n;
export const CLASS_ID_COMPONENT_GET_VALUES_RESPONSE = -2056830896783471569n;
export const CLASS_ID_COMPONENT_LIST_RESPONSE = 6592714266827556333n;
export const CLASS_ID_GET_COMPONENT_LIST_REQUEST = 6023810896618378113n;
export const CLASS_ID_GET_PROPERTY_DESCRIPTORS = 5150175307679594166n;
export const CLASS_ID_PROPERTY_DESCRIPTOR_RESPONSE = 6963368381922795052n;
export const CLASS_ID_PROPERTY_GET_VALUE_REQUEST = 901735839132130952n;
export const CLASS_ID_PROPERTY_GET_VALUE_RESPONSE = 8945416611226246596n;
export const CLASS_ID_PROPERTY_SET_VALUE_REQUEST = -1776666409656556548n;
export const CLASS_ID_PROPERTY_SET_VALUE_RESPONSE = 1995658389030653232n;
export const CLASS_ID_SET_PROPERTY_NOTIFY_MODE = -7219101800050484750n;
export const CLASS_ID_SYSTEM_GET_VALUES_REQUEST = -8302559874499546923n;
export const CLASS_ID_SYSTEM_GET_VALUES_RESPONSE = -688309113082437205n;

export class ComponentGetValuesRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = 0;

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' }
  ];
}

export class ComponentGetValuesResponse extends MikanResponse {
  ownerSystem: string = '';
  componentClassName: string = '';
  valuesObject: PolymorphicObject = new PolymorphicObject();

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentClassName', type: 'string' },
    { name: 'valuesObject', type: 'PolymorphicObject' }
  ];
}

export class ComponentListResponse extends MikanResponse {
  componentIdList: number[] = [];

  static __serializationMetadata = [
    { name: 'componentIdList', type: 'int32', isArray: true }
  ];
}

export class GetComponentListRequest extends MikanRequest {
  ownerSystem: string = '';
  componentClassName: string = '';

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentClassName', type: 'string' }
  ];
}

export class GetPropertyDescriptors extends MikanRequest {
  systemFilter: string = '';
  componentFilter: string = '';
  propertyFilter: string = '';

  static __serializationMetadata = [
    { name: 'systemFilter', type: 'string' },
    { name: 'componentFilter', type: 'string' },
    { name: 'propertyFilter', type: 'string' }
  ];
}

export class PropertyDescriptorResponse extends MikanResponse {
  descriptor_list: MikanPropertyDescriptor[] = [];

  static __serializationMetadata = [
    { name: 'descriptor_list', type: 'MikanPropertyDescriptor', isArray: true }
  ];
}

export class PropertyGetValueRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = 0;
  fieldName: string = '';

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'fieldName', type: 'string' }
  ];
}

export class PropertyGetValueResponse extends MikanResponse {
  propertyValue: MikanPropertyValue = new MikanPropertyValue();

  static __serializationMetadata = [
    { name: 'propertyValue', type: 'MikanPropertyValue' }
  ];
}

export class PropertySetValueRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = 0;
  fieldName: string = '';
  fieldValue: MikanVariant = new MikanVariant();

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'fieldName', type: 'string' },
    { name: 'fieldValue', type: 'MikanVariant' }
  ];
}

export class PropertySetValueResponse extends MikanResponse {

  static __serializationMetadata = [
  ];
}

export class SetPropertyNotifyMode extends MikanRequest {
  systemFilter: string = '';
  componentFilter: string = '';
  propertyFilter: string = '';
  notifyMode: MikanPropertyNotifyMode = 0;

  static __serializationMetadata = [
    { name: 'systemFilter', type: 'string' },
    { name: 'componentFilter', type: 'string' },
    { name: 'propertyFilter', type: 'string' },
    { name: 'notifyMode', type: 'enum:MikanPropertyNotifyMode' }
  ];
}

export class SystemGetValuesRequest extends MikanRequest {
  ownerSystem: string = '';

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' }
  ];
}

export class SystemGetValuesResponse extends MikanResponse {
  ownerSystem: string = '';
  valuesObject: PolymorphicObject = new PolymorphicObject();

  static __serializationMetadata = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'valuesObject', type: 'PolymorphicObject' }
  ];
}

