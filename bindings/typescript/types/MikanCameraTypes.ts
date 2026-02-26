// This file is auto generated. DO NOT EDIT.

import { MikanQuatd, MikanVector3d } from './MikanMathTypes';
import { MikanTransformComponentValues } from './MikanTransformTypes';

export const CLASS_ID_MIKAN_CAMERA_COMPONENT_VALUES = -1280596257660192689n;

export class MikanCameraComponentValues extends MikanTransformComponentValues {
  stage_id: number = 0;
  tracking_mount_id: number = 0;
  video_source_id: number = 0;
  tracking_frame_delay: number = 0;
  aperture_orientation_offset: MikanQuatd = new MikanQuatd();
  aperture_position_offset: MikanVector3d = new MikanVector3d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'stage_id', type: 'int32' },
    { name: 'tracking_mount_id', type: 'int32' },
    { name: 'video_source_id', type: 'int32' },
    { name: 'tracking_frame_delay', type: 'int32' },
    { name: 'aperture_orientation_offset', type: 'MikanQuatd' },
    { name: 'aperture_position_offset', type: 'MikanVector3d' }
  ];
}

