// This file is auto generated. DO NOT EDIT.

import { MikanSystemValues } from './MikanPropertyTypes.js';
import { MikanTransformComponentValues } from './MikanTransformTypes.js';

export class MikanSceneSystemValues extends MikanSystemValues {
  current_scene_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'current_scene_id', type: 'int32' }
  ];
}

export class MikanSceneComponentValues extends MikanTransformComponentValues {
  parent_stage_id: number = -1;
  display_compositor_id: number = -1;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'parent_stage_id', type: 'int32' },
    { name: 'display_compositor_id', type: 'int32' }
  ];
}

