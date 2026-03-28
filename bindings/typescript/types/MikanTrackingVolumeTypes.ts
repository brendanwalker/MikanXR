// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';
import { MikanMatrix4f, MikanVector3f } from './MikanMathTypes.js';

export enum MikanTrackingRuntime {
  INVALID = -1,
  SteamVR = 0
}

export enum MikanTrackingVolumeType {
  INVALID = -1,
  marker = 0,
  vr = 1
}

export class MikanTrackingVolumeComponentValues extends MikanComponentValues {
  origin_marker_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'origin_marker_id', type: 'int32' }
  ];
}

export class MikanVRTrackingVolumeComponentValues extends MikanTrackingVolumeComponentValues {
  tracking_runtime: MikanTrackingRuntime = MikanTrackingRuntime.INVALID;
  charuco_mount_id: number = -1;
  charuco_mount_offset_mm: MikanVector3f = new MikanVector3f();
  utility_marker_id: number = -1;
  tracking_mount_ids: number[] = [];
  vr_device_pose_offset: MikanMatrix4f = new MikanMatrix4f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'tracking_runtime', type: 'enum:MikanTrackingRuntime' },
    { name: 'charuco_mount_id', type: 'int32' },
    { name: 'charuco_mount_offset_mm', type: 'MikanVector3f' },
    { name: 'utility_marker_id', type: 'int32' },
    { name: 'tracking_mount_ids', type: 'int32', isArray: true },
    { name: 'vr_device_pose_offset', type: 'MikanMatrix4f' }
  ];
}

export class MikanMarkerTrackingVolumeComponentValues extends MikanTrackingVolumeComponentValues {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

