// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes.js';
import { MikanSystemValues } from './MikanPropertyTypes.js';

export enum MikanMarkerDictionaryType {
  INVALID = -1,
  DICT_4x4 = 0,
  DICT_5x5 = 1,
  DICT_6x6 = 2,
  DICT_7x7 = 3
}

export const CLASS_ID_MIKAN_MARKER_COMPONENT_VALUES = 2808287499176319284n;
export const CLASS_ID_MIKAN_MARKER_SYSTEM_VALUES = 133061264921636720n;

export class MikanMarkerComponentValues extends MikanComponentValues {
  aruco_id: number = -1;
  length_mm: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'aruco_id', type: 'int32' },
    { name: 'length_mm', type: 'float' }
  ];
}

export class MikanMarkerSystemValues extends MikanSystemValues {
  aruco_id_list: number[] = [];
  aruco_dictionary_type: MikanMarkerDictionaryType = MikanMarkerDictionaryType.DICT_4x4;
  charuco_rows: number = 0;
  charuco_cols: number = 0;
  charuco_square_length_mm: number = 0;
  charuco_marker_length_mm: number = 0;
  charuco_dictionary_type: MikanMarkerDictionaryType = MikanMarkerDictionaryType.INVALID;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'aruco_id_list', type: 'int32', isArray: true },
    { name: 'aruco_dictionary_type', type: 'enum:MikanMarkerDictionaryType' },
    { name: 'charuco_rows', type: 'int32' },
    { name: 'charuco_cols', type: 'int32' },
    { name: 'charuco_square_length_mm', type: 'float' },
    { name: 'charuco_marker_length_mm', type: 'float' },
    { name: 'charuco_dictionary_type', type: 'enum:MikanMarkerDictionaryType' }
  ];
}

