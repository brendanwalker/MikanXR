// This file is auto generated. DO NOT EDIT.

import { MikanTransformComponentValues } from './MikanTransformTypes';

export enum MikanStageTrackingVolume {
  StaticMarker = 0,
  SteamVR = 1
}

export const CLASS_ID_MIKAN_STAGE_COMPONENT_VALUES = -4271948442370858806n;

export class MikanStageComponentValues extends MikanTransformComponentValues {
  tracking_volume_id: number = 0;

  static __serializationMetadata = [
    { name: 'tracking_volume_id', type: 'int32' }
  ];
}

