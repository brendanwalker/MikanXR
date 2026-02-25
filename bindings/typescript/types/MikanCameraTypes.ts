// This file is auto generated. DO NOT EDIT.

import { MikanQuatd, MikanVector3d } from './MikanMathTypes';
import { MikanTransformComponentValues } from './MikanTransformTypes';

export const CLASS_ID_MIKAN_CAMERA_COMPONENT_VALUES = -1280596257660192689n;

export interface MikanCameraComponentValues extends MikanTransformComponentValues {
  stage_id: number;
  tracking_mount_id: number;
  video_source_id: number;
  tracking_frame_delay: number;
  aperture_orientation_offset: MikanQuatd;
  aperture_position_offset: MikanVector3d;
}

