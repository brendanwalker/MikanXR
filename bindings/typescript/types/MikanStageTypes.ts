// This file is auto generated. DO NOT EDIT.

import { MikanTransformComponentValues } from './MikanTransformTypes.js';
import type { SerializationField } from './SerializationTypes.js';

export enum MikanStageTrackingVolume {
  StaticMarker = 0,
  SteamVR = 1
}

export class MikanStageComponentValues extends MikanTransformComponentValues {
  tracking_volume_id: number = -1;

  static __serializationMetadata: SerializationField[] = [
    { name: 'tracking_volume_id', type: 'int32' }
  ];
}

