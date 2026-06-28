// This file is auto generated. DO NOT EDIT.

import { MikanSystemValues } from './MikanPropertyTypes.js';
import { MikanTransformComponentValues } from './MikanTransformTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export enum MikanDMXBufferFormat {
  DMXUncompressed = 0,
  DMXRLEEncoded = 1
}

export class MikanDMXData {
  server_time_seconds: number = 0;
  universes: MikanUniverseDMXData[] = [];

  static __serializationMetadata: SerializationField[] = [
    { name: 'server_time_seconds', type: 'double' },
    { name: 'universes', type: 'MikanUniverseDMXData', isArray: true }
  ];
}

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

export class MikanUniverseDMXData {
  dmx_universe_id: number = 0;
  buffer_format: MikanDMXBufferFormat = MikanDMXBufferFormat.DMXUncompressed;
  buffer_data: number[] = [];

  static __serializationMetadata: SerializationField[] = [
    { name: 'dmx_universe_id', type: 'uint16' },
    { name: 'buffer_format', type: 'enum:MikanDMXBufferFormat' },
    { name: 'buffer_data', type: 'uint8', isArray: true }
  ];
}

export class MikanRGBPixelGridSystemValues extends MikanSystemValues {

  static __serializationMetadata: SerializationField[] = [
  ];
}

export class MikanDMXFixtureComponentValues extends MikanTransformComponentValues {
  stage_id: number = -1;
  dmx_universe: number = 1;
  dmx_start_channel: number = 1;
  dmx_channel_count: number = 3;
  is_disabled: boolean = false;

  static __serializationMetadata: SerializationField[] = [
    { name: 'stage_id', type: 'int32' },
    { name: 'dmx_universe', type: 'uint16' },
    { name: 'dmx_start_channel', type: 'uint16' },
    { name: 'dmx_channel_count', type: 'uint16' },
    { name: 'is_disabled', type: 'boolean' }
  ];
}

export class MikanRGBSpotLightComponentValues extends MikanDMXFixtureComponentValues {
  cone_angle_degrees: number = 0;
  cone_range_meters: number = 0;

  static __serializationMetadata: SerializationField[] = [
    { name: 'cone_angle_degrees', type: 'float' },
    { name: 'cone_range_meters', type: 'float' }
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

