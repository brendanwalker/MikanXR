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

export interface ComponentGetValuesRequest extends MikanRequest {
  ownerSystem: string;
  componentId: number;
}

export interface ComponentGetValuesResponse extends MikanResponse {
  ownerSystem: string;
  componentClassName: string;
  valuesObject: PolymorphicObject;
}

export interface ComponentListResponse extends MikanResponse {
  componentIdList: number[];
}

export interface GetComponentListRequest extends MikanRequest {
  ownerSystem: string;
  componentClassName: string;
}

export interface GetPropertyDescriptors extends MikanRequest {
  systemFilter: string;
  componentFilter: string;
  propertyFilter: string;
}

export interface PropertyDescriptorResponse extends MikanResponse {
  descriptor_list: MikanPropertyDescriptor[];
}

export interface PropertyGetValueRequest extends MikanRequest {
  ownerSystem: string;
  componentId: number;
  fieldName: string;
}

export interface PropertyGetValueResponse extends MikanResponse {
  propertyValue: MikanPropertyValue;
}

export interface PropertySetValueRequest extends MikanRequest {
  ownerSystem: string;
  componentId: number;
  fieldName: string;
  fieldValue: MikanVariant;
}

export interface PropertySetValueResponse extends MikanResponse {
}

export interface SetPropertyNotifyMode extends MikanRequest {
  systemFilter: string;
  componentFilter: string;
  propertyFilter: string;
  notifyMode: MikanPropertyNotifyMode;
}

export interface SystemGetValuesRequest extends MikanRequest {
  ownerSystem: string;
}

export interface SystemGetValuesResponse extends MikanResponse {
  ownerSystem: string;
  valuesObject: PolymorphicObject;
}

