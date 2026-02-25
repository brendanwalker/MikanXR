// This file is auto generated. DO NOT EDIT.

import { PolymorphicObject, PolymorphicStruct } from './../PolymorphicObject';
import { MikanComponentValues } from './MikanComponentTypes';
import { MikanMatrix3d, MikanMatrix4d, MikanMatrix4x3d, MikanVector3d } from './MikanMathTypes';

export enum MikanIntrinsicsType {
  INVALID = 0,
  MONO_CAMERA_INTRINSICS = 1,
  STEREO_CAMERA_INTRINSICS = 2
}

export enum MikanVideoSettingType {
  INVALID = -1,
  Brightness = 0,
  Contrast = 1,
  Hue = 2,
  Saturation = 3,
  Sharpness = 4,
  Gamma = 5,
  WhiteBalance = 6,
  RedBalance = 7,
  GreenBalance = 8,
  BlueBalance = 9,
  Gain = 10,
  Pan = 11,
  Tilt = 12,
  Roll = 13,
  Zoom = 14,
  Exposure = 15,
  Iris = 16,
  Focus = 17,
  Count = 18
}

export enum MikanVideoSourceType {
  MONO = 0,
  STEREO = 1
}

export const CLASS_ID_MIKAN_BASE_INTRINSICS = -3286470658648308984n;
export const CLASS_ID_MIKAN_DISTORTION_COEFFICIENTS = -2596555002374434624n;
export const CLASS_ID_MIKAN_MONO_INTRINSICS = 4896055255137140914n;
export const CLASS_ID_MIKAN_NETWORK_VIDEO_SOURCE_VALUES = 8924237416134720747n;
export const CLASS_ID_MIKAN_STEREO_INTRINSICS = -261934067861644075n;
export const CLASS_ID_MIKAN_USBVIDEO_SOURCE_VALUES = -9166828371246079689n;
export const CLASS_ID_MIKAN_VARIANT_BASE = 5706978007370628991n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_INTRINSICS = -5073913459979558727n;
export const CLASS_ID_MIKAN_VIDEO_SOURCE_VALUES = -7299893175604117141n;

export interface MikanBaseIntrinsics extends PolymorphicStruct {
  pixel_width: number;
  pixel_height: number;
  aspect_ratio: number;
  hfov: number;
  vfov: number;
  znear: number;
  zfar: number;
}

export interface MikanDistortionCoefficients {
  k1: number;
  k2: number;
  k3: number;
  k4: number;
  k5: number;
  k6: number;
  p1: number;
  p2: number;
}

export interface MikanMonoIntrinsics extends MikanBaseIntrinsics {
  distortion_coefficients: MikanDistortionCoefficients;
  distorted_camera_matrix: MikanMatrix3d;
  undistorted_camera_matrix: MikanMatrix3d;
}

export interface MikanNetworkVideoSourceValues extends MikanVideoSourceValues {
  protocol: string;
  ip_address: string;
  port: number;
  path: string;
}

export interface MikanStereoIntrinsics extends MikanBaseIntrinsics {
  left_distortion_coefficients: MikanDistortionCoefficients;
  left_camera_matrix: MikanMatrix3d;
  right_distortion_coefficients: MikanDistortionCoefficients;
  right_camera_matrix: MikanMatrix3d;
  left_rectification_rotation: MikanMatrix3d;
  right_rectification_rotation: MikanMatrix3d;
  left_rectification_projection: MikanMatrix4x3d;
  right_rectification_projection: MikanMatrix4x3d;
  rotation_between_cameras: MikanMatrix3d;
  translation_between_cameras: MikanVector3d;
  essential_matrix: MikanMatrix3d;
  fundamental_matrix: MikanMatrix3d;
  reprojection_matrix: MikanMatrix4d;
}

export interface MikanUSBVideoSourceValues extends MikanVideoSourceValues {
  current_device_path: string;
  video_mode: string;
  video_settings: number[];
}

export interface MikanVariantBase extends PolymorphicStruct {
}

export interface MikanVideoSourceIntrinsics {
  intrinsics_ptr: PolymorphicObject;
  intrinsics_type: MikanIntrinsicsType;
}

export interface MikanVideoSourceValues extends MikanComponentValues {
  intrinsics_ptr: PolymorphicObject;
  intrinsics_type: MikanIntrinsicsType;
  is_frame_mirrored: boolean;
  is_buffer_mirrored: boolean;
  video_frame_queue_size: number;
}

