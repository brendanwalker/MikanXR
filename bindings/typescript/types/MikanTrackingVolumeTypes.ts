// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes';
import { MikanMatrix4f, MikanVector3f } from './MikanMathTypes';

export enum MikanTrackingRuntime {
  INVALID = -1,
  SteamVR = 0
}

export enum MikanTrackingVolumeType {
  INVALID = -1,
  marker = 0,
  vr = 1
}

export const CLASS_ID_MIKAN_MARKER_TRACKING_VOLUME_COMPONENT_VALUES = -912859617642764411n;
export const CLASS_ID_MIKAN_TRACKING_VOLUME_COMPONENT_VALUES = 8298175138156239447n;
export const CLASS_ID_MIKAN_VRTRACKING_VOLUME_COMPONENT_VALUES = -4734525919723628573n;

export interface MikanMarkerTrackingVolumeComponentValues extends MikanTrackingVolumeComponentValues {
}

export interface MikanTrackingVolumeComponentValues extends MikanComponentValues {
  origin_marker_id: number;
}

export interface MikanVRTrackingVolumeComponentValues extends MikanTrackingVolumeComponentValues {
  tracking_runtime: MikanTrackingRuntime;
  charuco_mount_id: number;
  charuco_mount_offset_mm: MikanVector3f;
  utility_marker_id: number;
  tracking_mount_ids: number[];
  vr_device_pose_offset: MikanMatrix4f;
}

