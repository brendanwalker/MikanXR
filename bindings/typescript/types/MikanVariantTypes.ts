// This file is auto generated. DO NOT EDIT.

import { PolymorphicObject, PolymorphicStruct } from './../PolymorphicObject';
import { MikanMatrix4f, MikanQuatd, MikanQuatf, MikanVector2d, MikanVector2f, MikanVector3d, MikanVector3f, MikanVector4d, MikanVector4f } from './MikanMathTypes';
import { MikanVariantBase } from './MikanVideoSourceTypes';

export enum MikanVariantType {
  INVALID_TYPE = 0,
  BOOL_TYPE = 1,
  INT_TYPE = 2,
  LONG_TYPE = 3,
  FLOAT_TYPE = 4,
  DOUBLE_TYPE = 5,
  MK_STRING_TYPE = 6,
  VECTOR2F_TYPE = 7,
  VECTOR3F_TYPE = 8,
  VECTOR4F_TYPE = 9,
  QUATERNIONF_TYPE = 10,
  MATRIX4F_TYPE = 11,
  VECTOR2D_TYPE = 12,
  VECTOR3D_TYPE = 13,
  VECTOR4D_TYPE = 14,
  QUATERNIOND_TYPE = 15,
  BOOL_ARRAY_TYPE = 16,
  INT_ARRAY_TYPE = 17,
  FLOAT_ARRAY_TYPE = 18,
  POLYMORPHIC_OBJECT_TYPE = 19
}

export const CLASS_ID_MIKAN_BOOL_ARRAY_VALUE = -7544815710777554173n;
export const CLASS_ID_MIKAN_BOOL_VALUE = 2219001884115983454n;
export const CLASS_ID_MIKAN_DOUBLE_VALUE = 8832589766978231013n;
export const CLASS_ID_MIKAN_FLOAT_ARRAY_VALUE = 497562989614948859n;
export const CLASS_ID_MIKAN_FLOAT_VALUE = 595971514476674198n;
export const CLASS_ID_MIKAN_INT_ARRAY_VALUE = -2537235331547746544n;
export const CLASS_ID_MIKAN_INT_VALUE = 6192287855589871355n;
export const CLASS_ID_MIKAN_LONG_VALUE = -2202905653303995628n;
export const CLASS_ID_MIKAN_MATRIX4F_VALUE = -2194723481896302537n;
export const CLASS_ID_MIKAN_QUATD_VALUE = -303519628627493629n;
export const CLASS_ID_MIKAN_QUATF_VALUE = -3980278345028742771n;
export const CLASS_ID_MIKAN_STRING_VALUE = -318636760475246811n;
export const CLASS_ID_MIKAN_VARIANT = -8543347830987565886n;
export const CLASS_ID_MIKAN_VECTOR2D_VALUE = -3597335519782313151n;
export const CLASS_ID_MIKAN_VECTOR2F_VALUE = 5487063928969330551n;
export const CLASS_ID_MIKAN_VECTOR3D_VALUE = 905231883766189926n;
export const CLASS_ID_MIKAN_VECTOR3F_VALUE = 3503799446698825680n;
export const CLASS_ID_MIKAN_VECTOR4D_VALUE = 398000504403518783n;
export const CLASS_ID_MIKAN_VECTOR4F_VALUE = -1402910374681859671n;

export interface MikanBoolArrayValue extends MikanVariantBase {
  value: boolean[];
}

export interface MikanBoolValue extends MikanVariantBase {
  value: boolean;
}

export interface MikanDoubleValue extends MikanVariantBase {
  value: number;
}

export interface MikanFloatArrayValue extends MikanVariantBase {
  value: number[];
}

export interface MikanFloatValue extends MikanVariantBase {
  value: number;
}

export interface MikanIntArrayValue extends MikanVariantBase {
  value: number[];
}

export interface MikanIntValue extends MikanVariantBase {
  value: number;
}

export interface MikanLongValue extends MikanVariantBase {
  value: any;
}

export interface MikanMatrix4fValue extends MikanVariantBase {
  value: MikanMatrix4f;
}

export interface MikanQuatdValue extends MikanVariantBase {
  value: MikanQuatd;
}

export interface MikanQuatfValue extends MikanVariantBase {
  value: MikanQuatf;
}

export interface MikanStringValue extends MikanVariantBase {
  value: string;
}

export interface MikanVariant {
  value_type: MikanVariantType;
  value_ptr: PolymorphicObject;
}

export interface MikanVector2dValue extends MikanVariantBase {
  value: MikanVector2d;
}

export interface MikanVector2fValue extends MikanVariantBase {
  value: MikanVector2f;
}

export interface MikanVector3dValue extends MikanVariantBase {
  value: MikanVector3d;
}

export interface MikanVector3fValue extends MikanVariantBase {
  value: MikanVector3f;
}

export interface MikanVector4dValue extends MikanVariantBase {
  value: MikanVector4d;
}

export interface MikanVector4fValue extends MikanVariantBase {
  value: MikanVector4f;
}

