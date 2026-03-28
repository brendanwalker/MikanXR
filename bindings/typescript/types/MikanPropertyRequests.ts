// This file is auto generated. DO NOT EDIT.

import { PolymorphicObject, PolymorphicStruct } from './../PolymorphicObject.js';
import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanPropertyDescriptor, MikanPropertyValue } from './MikanPropertyTypes.js';
import { MikanVariant } from './MikanVariantTypes.js';

export enum MikanPropertyNotifyMode {
  NONE = 0,
  NAME = 1,
  NAME_AND_VALUE = 2
}

export class SetPropertyNotifyMode extends MikanRequest {
  systemFilter: string = '';
  componentFilter: string = '';
  propertyFilter: string = '';
  notifyMode: MikanPropertyNotifyMode = MikanPropertyNotifyMode.NONE;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'systemFilter', type: 'string' },
    { name: 'componentFilter', type: 'string' },
    { name: 'propertyFilter', type: 'string' },
    { name: 'notifyMode', type: 'enum:MikanPropertyNotifyMode' }
  ];
}

export class ComponentGetValuesResponse extends MikanResponse {
  ownerSystem: string = '';
  componentClassName: string = '';
  valuesObject: PolymorphicObject = new PolymorphicObject();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentClassName', type: 'string' },
    { name: 'valuesObject', type: 'PolymorphicObject' }
  ];
}

export class SystemCreateObjectRequest extends MikanRequest {
  ownerSystem: string = '';
  componentClassName: string = '';
  initParams: PolymorphicObject = new PolymorphicObject();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentClassName', type: 'string' },
    { name: 'initParams', type: 'PolymorphicObject' }
  ];
}

export class PropertySetValueRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = -1;
  fieldName: string = '';
  fieldValue: MikanVariant = new MikanVariant();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'fieldName', type: 'string' },
    { name: 'fieldValue', type: 'MikanVariant' }
  ];
}

export class GetComponentListRequest extends MikanRequest {
  ownerSystem: string = '';
  componentClassName: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentClassName', type: 'string' }
  ];
}

export class ComponentGetValuesRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' }
  ];
}

export class PropertyGetValueRequest extends MikanRequest {
  ownerSystem: string = '';
  componentId: number = -1;
  fieldName: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentId', type: 'int32' },
    { name: 'fieldName', type: 'string' }
  ];
}

export class SystemGetValuesRequest extends MikanRequest {
  ownerSystem: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' }
  ];
}

export class SystemDestroyObjectRequest extends MikanRequest {
  ownerSystem: string = '';
  componentClassName: string = '';
  componentId: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'componentClassName', type: 'string' },
    { name: 'componentId', type: 'int32' }
  ];
}

export class GetPropertyDescriptors extends MikanRequest {
  systemFilter: string = '';
  componentFilter: string = '';
  propertyFilter: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'systemFilter', type: 'string' },
    { name: 'componentFilter', type: 'string' },
    { name: 'propertyFilter', type: 'string' }
  ];
}

export class PropertySetValueResponse extends MikanResponse {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class PropertyGetValueResponse extends MikanResponse {
  propertyValue: MikanPropertyValue = new MikanPropertyValue();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'propertyValue', type: 'MikanPropertyValue' }
  ];
}

export class ComponentListResponse extends MikanResponse {
  componentIdList: number[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'componentIdList', type: 'int32', isArray: true }
  ];
}

export class SystemGetValuesResponse extends MikanResponse {
  ownerSystem: string = '';
  valuesObject: PolymorphicObject = new PolymorphicObject();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'ownerSystem', type: 'string' },
    { name: 'valuesObject', type: 'PolymorphicObject' }
  ];
}

export class PropertyDescriptorResponse extends MikanResponse {
  descriptor_list: MikanPropertyDescriptor[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'descriptor_list', type: 'MikanPropertyDescriptor', isArray: true }
  ];
}

