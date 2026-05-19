// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanRGBPixelGridComponentValues, MikanRGBSpotLightComponentValues } from './MikanLightTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class DestroyRGBPixelGrid extends MikanRequest {
  grid_id: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'DestroyRGBPixelGrid';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_id', type: 'int32' }
  ];
}

export class MikanRGBSpotLightResponse extends MikanResponse {
  light_info: MikanRGBSpotLightComponentValues = new MikanRGBSpotLightComponentValues();

  constructor() {
    super();
    this.responseTypeName = 'MikanRGBSpotLightResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_info', type: 'MikanRGBSpotLightComponentValues' }
  ];
}

export class GetRGBPixelGridList extends MikanRequest {

  constructor() {
    super();
    this.requestTypeName = 'GetRGBPixelGridList';
  }

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanRGBPixelGridResponse extends MikanResponse {
  grid_info: MikanRGBPixelGridComponentValues = new MikanRGBPixelGridComponentValues();

  constructor() {
    super();
    this.responseTypeName = 'MikanRGBPixelGridResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_info', type: 'MikanRGBPixelGridComponentValues' }
  ];
}

export class CreateRGBSpotLight extends MikanRequest {

  constructor() {
    super();
    this.requestTypeName = 'CreateRGBSpotLight';
  }

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanRGBPixelGridListResponse extends MikanResponse {
  grid_ids: number[] = [];

  constructor() {
    super();
    this.responseTypeName = 'MikanRGBPixelGridListResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_ids', type: 'int32', isArray: true }
  ];
}

export class MikanRGBSpotLightListResponse extends MikanResponse {
  light_ids: number[] = [];

  constructor() {
    super();
    this.responseTypeName = 'MikanRGBSpotLightListResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_ids', type: 'int32', isArray: true }
  ];
}

export class GetRGBSpotLightList extends MikanRequest {

  constructor() {
    super();
    this.requestTypeName = 'GetRGBSpotLightList';
  }

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class DestroyRGBSpotLight extends MikanRequest {
  light_id: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'DestroyRGBSpotLight';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' }
  ];
}

export class GetRGBSpotLight extends MikanRequest {
  light_id: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'GetRGBSpotLight';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' }
  ];
}

export class CreateRGBPixelGrid extends MikanRequest {

  constructor() {
    super();
    this.requestTypeName = 'CreateRGBPixelGrid';
  }

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class GetRGBPixelGrid extends MikanRequest {
  grid_id: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'GetRGBPixelGrid';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_id', type: 'int32' }
  ];
}

export class SetRGBPixelGridData extends MikanRequest {
  grid_id: number = -1;
  pixel_data: number[] = [];

  constructor() {
    super();
    this.requestTypeName = 'SetRGBPixelGridData';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_id', type: 'int32' },
    { name: 'pixel_data', type: 'uint8', isArray: true }
  ];
}

