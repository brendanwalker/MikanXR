// This file is auto generated. DO NOT EDIT.

import { MikanSystemValues } from './MikanPropertyTypes.js';
import { MikanTransformComponentValues } from './MikanTransformTypes.js';

export const CLASS_ID_MIKAN_SCENE_COMPONENT_VALUES = 6935017739381736592n;
export const CLASS_ID_MIKAN_SCENE_SYSTEM_VALUES = 1136621147171447108n;

export class MikanSceneComponentValues extends MikanTransformComponentValues {
  parent_stage_id: number = -1;
  display_compositor_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'parent_stage_id', type: 'int32' },
    { name: 'display_compositor_id', type: 'int32' }
  ];
}

export class MikanSceneSystemValues extends MikanSystemValues {
  current_scene_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'current_scene_id', type: 'int32' }
  ];
}

