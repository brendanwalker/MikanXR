// This file is auto generated. DO NOT EDIT.

import { MikanTransformComponentValues } from './MikanTransformTypes.js';

export enum MikanStageTrackingVolume {
  StaticMarker = 0,
  SteamVR = 1
}

export const CLASS_ID_MIKAN_STAGE_COMPONENT_VALUES = -4271948442370858806n;

export class MikanStageComponentValues extends MikanTransformComponentValues {
  tracking_volume_id: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'tracking_volume_id', type: 'int32' }
  ];
}

