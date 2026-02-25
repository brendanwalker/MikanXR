// This file is auto generated. DO NOT EDIT.

import { MikanEvent } from './MikanAPITypes';
import { MikanVector2d, MikanVector2i, MikanVector3f } from './MikanMathTypes';

export const CLASS_ID_MIKAN_CAMERA_NEW_FRAME_EVENT = -514294351884360997n;

export interface MikanCameraNewFrameEvent extends MikanEvent {
  camera_id: number;
  camera_forward: MikanVector3f;
  camera_up: MikanVector3f;
  camera_position: MikanVector3f;
  pixel_size: MikanVector2i;
  focal_length: MikanVector2d;
  principal_point: MikanVector2d;
  z_bounds: MikanVector2d;
  frame: bigint;
}

