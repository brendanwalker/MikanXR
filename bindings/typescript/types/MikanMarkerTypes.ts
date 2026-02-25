// This file is auto generated. DO NOT EDIT.

import { MikanComponentValues } from './MikanComponentTypes';
import { MikanSystemValues } from './MikanPropertyTypes';

export enum MikanMarkerDictionaryType {
  DICT_4x4 = 0,
  DICT_5x5 = 1,
  DICT_6x6 = 2,
  DICT_7x7 = 3
}

export const CLASS_ID_MIKAN_MARKER_COMPONENT_VALUES = 2808287499176319284n;
export const CLASS_ID_MIKAN_MARKER_SYSTEM_VALUES = 133061264921636720n;

export interface MikanMarkerComponentValues extends MikanComponentValues {
  aruco_id: number;
  length_mm: number;
}

export interface MikanMarkerSystemValues extends MikanSystemValues {
  aruco_id_list: number[];
  aruco_dictionary_type: MikanMarkerDictionaryType;
  charuco_rows: number;
  charuco_cols: number;
  charuco_square_length_mm: number;
  charuco_marker_length_mm: number;
  charuco_dictionary_type: MikanMarkerDictionaryType;
}

