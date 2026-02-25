// This file is auto generated. DO NOT EDIT.

import { MikanTransformComponentValues } from './MikanTransformTypes';

export enum MikanVRDeviceApi {
  INVALID = 0,
  STEAM_VR = 1
}

export enum MikanVRDeviceType {
  INVALID = 0,
  HMD = 1,
  CONTROLLER = 2,
  TRACKER = 3
}

export const CLASS_ID_MIKAN_VRDEVICE_COMPONENT_VALUES = 5417189174165719684n;

export interface MikanVRDeviceComponentValues extends MikanTransformComponentValues {
  vr_device_api: MikanVRDeviceApi;
  vr_device_type: MikanVRDeviceType;
  vr_device_index: number;
  vr_device_path: string;
}

