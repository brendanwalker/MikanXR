// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanRGBPixelGridDeletedEvent extends MikanEvent {
  grid_id: number = -1;

  constructor() {
    super();
    this.eventTypeName = 'MikanRGBPixelGridDeletedEvent';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_id', type: 'int32' }
  ];
}

export class MikanRGBSpotLightCreatedEvent extends MikanEvent {
  light_id: number = -1;

  constructor() {
    super();
    this.eventTypeName = 'MikanRGBSpotLightCreatedEvent';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' }
  ];
}

export class MikanRGBSpotLightDeletedEvent extends MikanEvent {
  light_id: number = -1;

  constructor() {
    super();
    this.eventTypeName = 'MikanRGBSpotLightDeletedEvent';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'light_id', type: 'int32' }
  ];
}

export class MikanRGBPixelGridCreatedEvent extends MikanEvent {
  grid_id: number = -1;

  constructor() {
    super();
    this.eventTypeName = 'MikanRGBPixelGridCreatedEvent';
  }

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_id', type: 'int32' }
  ];
}

