// This file is auto generated. DO NOT EDIT.

import { MikanTransformComponentValues } from './MikanTransformTypes.js';

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

export class MikanVRDeviceComponentValues extends MikanTransformComponentValues {
  vr_device_api: MikanVRDeviceApi = MikanVRDeviceApi.INVALID;
  vr_device_type: MikanVRDeviceType = MikanVRDeviceType.INVALID;
  vr_device_index: number = -1;
  vr_device_path: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'vr_device_api', type: 'enum:MikanVRDeviceApi' },
    { name: 'vr_device_type', type: 'enum:MikanVRDeviceType' },
    { name: 'vr_device_index', type: 'int32' },
    { name: 'vr_device_path', type: 'string' }
  ];
}

