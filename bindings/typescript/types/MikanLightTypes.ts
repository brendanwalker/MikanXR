// This file is auto generated. DO NOT EDIT.

import { MikanSystemValues } from './MikanPropertyTypes.js';
import { MikanTransformComponentValues } from './MikanTransformTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export class MikanDMXObjectSystemValues extends MikanSystemValues {
  network_interface_ip: string = '';
  dmx_priority: number = 100;
  transmit_rate_hz: number = 44;

  static __serializationMetadata: SerializationField[] = [
    { name: 'network_interface_ip', type: 'string' },
    { name: 'dmx_priority', type: 'uint8' },
    { name: 'transmit_rate_hz', type: 'float' }
  ];
}

export class MikanRGBSpotLightSystemValues extends MikanSystemValues {

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanRGBPixelGridSystemValues extends MikanSystemValues {

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanDMXFixtureComponentValues extends MikanTransformComponentValues {
  dmx_universe: number = 1;
  dmx_start_channel: number = 1;
  dmx_channel_count: number = 3;
  is_disabled: boolean = false;

  static __serializationMetadata: SerializationField[] = [
    { name: 'dmx_universe', type: 'uint16' },
    { name: 'dmx_start_channel', type: 'uint16' },
    { name: 'dmx_channel_count', type: 'uint16' },
    { name: 'is_disabled', type: 'boolean' }
  ];
}

export class MikanRGBSpotLightComponentValues extends MikanDMXFixtureComponentValues {
  red: number = 0;
  green: number = 0;
  blue: number = 0;

  static __serializationMetadata: SerializationField[] = [
    { name: 'red', type: 'uint8' },
    { name: 'green', type: 'uint8' },
    { name: 'blue', type: 'uint8' }
  ];
}

export class MikanRGBPixelGridComponentValues extends MikanDMXFixtureComponentValues {
  grid_columns: number = 8;
  grid_rows: number = 8;

  static __serializationMetadata: SerializationField[] = [
    { name: 'grid_columns', type: 'int32' },
    { name: 'grid_rows', type: 'int32' }
  ];
}

