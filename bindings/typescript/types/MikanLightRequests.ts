// This file is auto generated. DO NOT EDIT.

import { MikanRequest, MikanResponse } from './MikanAPITypes.js';
import { MikanDMXData } from './MikanLightTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanLightDMXDataResponse extends MikanResponse {
  light_id: number = -1;
  dmx_data: MikanDMXData = new MikanDMXData();

  constructor() {
    super();
    this.responseTypeName = 'MikanLightDMXDataResponse';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' },
    { name: 'dmx_data', type: 'MikanDMXData' }
  ];
}

export class SetLightDMXData extends MikanRequest {
  light_id: number = -1;
  dmx_data: MikanDMXData = new MikanDMXData();

  constructor() {
    super();
    this.requestTypeName = 'SetLightDMXData';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' },
    { name: 'dmx_data', type: 'MikanDMXData' }
  ];
}

export class SetLightDMXDataSubcription extends MikanRequest {
  light_id: number = -1;
  subscribe: boolean = false;

  constructor() {
    super();
    this.requestTypeName = 'SetLightDMXDataSubcription';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' },
    { name: 'subscribe', type: 'boolean' }
  ];
}

export class GetLightDMXData extends MikanRequest {
  light_id: number = -1;

  constructor() {
    super();
    this.requestTypeName = 'GetLightDMXData';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' }
  ];
}

