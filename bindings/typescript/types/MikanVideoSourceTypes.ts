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

export class MikanBaseIntrinsics extends PolymorphicStruct {
  pixel_width: number = 0;
  pixel_height: number = 0;
  aspect_ratio: number = 0;
  hfov: number = 0;
  vfov: number = 0;
  znear: number = 0;
  zfar: number = 0;

  static __serializationMetadata = [
    { name: 'pixel_width', type: 'double' },
    { name: 'pixel_height', type: 'double' },
    { name: 'aspect_ratio', type: 'double' },
    { name: 'hfov', type: 'double' },
    { name: 'vfov', type: 'double' },
    { name: 'znear', type: 'double' },
    { name: 'zfar', type: 'double' }
  ];
}

export class MikanDistortionCoefficients {
  k1: number = 0;
  k2: number = 0;
  k3: number = 0;
  k4: number = 0;
  k5: number = 0;
  k6: number = 0;
  p1: number = 0;
  p2: number = 0;

  static __serializationMetadata = [
    { name: 'k1', type: 'double' },
    { name: 'k2', type: 'double' },
    { name: 'k3', type: 'double' },
    { name: 'k4', type: 'double' },
    { name: 'k5', type: 'double' },
    { name: 'k6', type: 'double' },
    { name: 'p1', type: 'double' },
    { name: 'p2', type: 'double' }
  ];
}

export class MikanMonoIntrinsics extends MikanBaseIntrinsics {
  distortion_coefficients: MikanDistortionCoefficients = new MikanDistortionCoefficients();
  distorted_camera_matrix: MikanMatrix3d = new MikanMatrix3d();
  undistorted_camera_matrix: MikanMatrix3d = new MikanMatrix3d();

  static __serializationMetadata = [
    { name: 'distortion_coefficients', type: 'MikanDistortionCoefficients' },
    { name: 'distorted_camera_matrix', type: 'MikanMatrix3d' },
    { name: 'undistorted_camera_matrix', type: 'MikanMatrix3d' }
  ];
}

export class MikanNetworkVideoSourceValues extends MikanVideoSourceValues {
  protocol: string = '';
  ip_address: string = '';
  port: number = 0;
  path: string = '';

  static __serializationMetadata = [
    { name: 'protocol', type: 'string' },
    { name: 'ip_address', type: 'string' },
    { name: 'port', type: 'int32' },
    { name: 'path', type: 'string' }
  ];
}

export class MikanStereoIntrinsics extends MikanBaseIntrinsics {
  left_distortion_coefficients: MikanDistortionCoefficients = new MikanDistortionCoefficients();
  left_camera_matrix: MikanMatrix3d = new MikanMatrix3d();
  right_distortion_coefficients: MikanDistortionCoefficients = new MikanDistortionCoefficients();
  right_camera_matrix: MikanMatrix3d = new MikanMatrix3d();
  left_rectification_rotation: MikanMatrix3d = new MikanMatrix3d();
  right_rectification_rotation: MikanMatrix3d = new MikanMatrix3d();
  left_rectification_projection: MikanMatrix4x3d = new MikanMatrix4x3d();
  right_rectification_projection: MikanMatrix4x3d = new MikanMatrix4x3d();
  rotation_between_cameras: MikanMatrix3d = new MikanMatrix3d();
  translation_between_cameras: MikanVector3d = new MikanVector3d();
  essential_matrix: MikanMatrix3d = new MikanMatrix3d();
  fundamental_matrix: MikanMatrix3d = new MikanMatrix3d();
  reprojection_matrix: MikanMatrix4d = new MikanMatrix4d();

  static __serializationMetadata = [
    { name: 'left_distortion_coefficients', type: 'MikanDistortionCoefficients' },
    { name: 'left_camera_matrix', type: 'MikanMatrix3d' },
    { name: 'right_distortion_coefficients', type: 'MikanDistortionCoefficients' },
    { name: 'right_camera_matrix', type: 'MikanMatrix3d' },
    { name: 'left_rectification_rotation', type: 'MikanMatrix3d' },
    { name: 'right_rectification_rotation', type: 'MikanMatrix3d' },
    { name: 'left_rectification_projection', type: 'MikanMatrix4x3d' },
    { name: 'right_rectification_projection', type: 'MikanMatrix4x3d' },
    { name: 'rotation_between_cameras', type: 'MikanMatrix3d' },
    { name: 'translation_between_cameras', type: 'MikanVector3d' },
    { name: 'essential_matrix', type: 'MikanMatrix3d' },
    { name: 'fundamental_matrix', type: 'MikanMatrix3d' },
    { name: 'reprojection_matrix', type: 'MikanMatrix4d' }
  ];
}

export class MikanUSBVideoSourceValues extends MikanVideoSourceValues {
  current_device_path: string = '';
  video_mode: string = '';
  video_settings: number[] = [];

  static __serializationMetadata = [
    { name: 'current_device_path', type: 'string' },
    { name: 'video_mode', type: 'string' },
    { name: 'video_settings', type: 'float', isArray: true }
  ];
}

export class MikanVariantBase extends PolymorphicStruct {

  static __serializationMetadata = [
  ];
}

export class MikanVideoSourceIntrinsics {
  intrinsics_ptr: PolymorphicObject = new PolymorphicObject();
  intrinsics_type: MikanIntrinsicsType = 0;

  static __serializationMetadata = [
    { name: 'intrinsics_ptr', type: 'PolymorphicObject' },
    { name: 'intrinsics_type', type: 'enum:MikanIntrinsicsType' }
  ];
}

export class MikanVideoSourceValues extends MikanComponentValues {
  intrinsics_ptr: PolymorphicObject = new PolymorphicObject();
  intrinsics_type: MikanIntrinsicsType = 0;
  is_frame_mirrored: boolean = false;
  is_buffer_mirrored: boolean = false;
  video_frame_queue_size: number = 0;

  static __serializationMetadata = [
    { name: 'intrinsics_ptr', type: 'PolymorphicObject' },
    { name: 'intrinsics_type', type: 'enum:MikanIntrinsicsType' },
    { name: 'is_frame_mirrored', type: 'boolean' },
    { name: 'is_buffer_mirrored', type: 'boolean' },
    { name: 'video_frame_queue_size', type: 'int32' }
  ];
}

