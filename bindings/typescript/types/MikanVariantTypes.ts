// This file is auto generated. DO NOT EDIT.

import { PolymorphicObject, PolymorphicStruct } from './../PolymorphicObject.js';
import { MikanMatrix4f, MikanQuatd, MikanQuatf, MikanVector2d, MikanVector2f, MikanVector3d, MikanVector3f, MikanVector4d, MikanVector4f } from './MikanMathTypes.js';

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
  STRING_ARRAY_TYPE = 19,
  STRING_MAP_TYPE = 20,
  POLYMORPHIC_OBJECT_TYPE = 21
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
export const CLASS_ID_MIKAN_STRING_ARRAY_VALUE = 5781178291974097454n;
export const CLASS_ID_MIKAN_STRING_MAP_VALUE = -5220301160931703877n;
export const CLASS_ID_MIKAN_STRING_VALUE = -318636760475246811n;
export const CLASS_ID_MIKAN_VARIANT = -8543347830987565886n;
export const CLASS_ID_MIKAN_VARIANT_BASE = 5706978007370628991n;
export const CLASS_ID_MIKAN_VECTOR2D_VALUE = -3597335519782313151n;
export const CLASS_ID_MIKAN_VECTOR2F_VALUE = 5487063928969330551n;
export const CLASS_ID_MIKAN_VECTOR3D_VALUE = 905231883766189926n;
export const CLASS_ID_MIKAN_VECTOR3F_VALUE = 3503799446698825680n;
export const CLASS_ID_MIKAN_VECTOR4D_VALUE = 398000504403518783n;
export const CLASS_ID_MIKAN_VECTOR4F_VALUE = -1402910374681859671n;

export class MikanVariantBase extends PolymorphicStruct {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class MikanBoolArrayValue extends MikanVariantBase {
  value: boolean[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'boolean' }
  ];
}

export class MikanBoolValue extends MikanVariantBase {
  value: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'boolean' }
  ];
}

export class MikanDoubleValue extends MikanVariantBase {
  value: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'double' }
  ];
}

export class MikanFloatArrayValue extends MikanVariantBase {
  value: number[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'float', isArray: true }
  ];
}

export class MikanFloatValue extends MikanVariantBase {
  value: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'float' }
  ];
}

export class MikanIntArrayValue extends MikanVariantBase {
  value: number[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'int32', isArray: true }
  ];
}

export class MikanIntValue extends MikanVariantBase {
  value: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'int32' }
  ];
}

export class MikanLongValue extends MikanVariantBase {
  value: any = null;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'int64' }
  ];
}

export class MikanMatrix4fValue extends MikanVariantBase {
  value: MikanMatrix4f = new MikanMatrix4f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanMatrix4f' }
  ];
}

export class MikanQuatdValue extends MikanVariantBase {
  value: MikanQuatd = new MikanQuatd();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanQuatd' }
  ];
}

export class MikanQuatfValue extends MikanVariantBase {
  value: MikanQuatf = new MikanQuatf();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanQuatf' }
  ];
}

export class MikanStringArrayValue extends MikanVariantBase {
  value: string[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'string', isArray: true }
  ];
}

export class MikanStringMapValue extends MikanVariantBase {
  value: Record<string, string> = {};

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'Map', isMap: true, keyType: 'string', valueType: 'string' }
  ];
}

export class MikanStringValue extends MikanVariantBase {
  value: string = '';

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'string' }
  ];
}

export class MikanVariant {
  value_type: MikanVariantType = MikanVariantType.INVALID_TYPE;
  value_ptr: PolymorphicObject = new PolymorphicObject();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value_type', type: 'enum:MikanVariantType' },
    { name: 'value_ptr', type: 'PolymorphicObject' }
  ];
}

export class MikanVector2dValue extends MikanVariantBase {
  value: MikanVector2d = new MikanVector2d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector2d' }
  ];
}

export class MikanVector2fValue extends MikanVariantBase {
  value: MikanVector2f = new MikanVector2f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector2f' }
  ];
}

export class MikanVector3dValue extends MikanVariantBase {
  value: MikanVector3d = new MikanVector3d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector3d' }
  ];
}

export class MikanVector3fValue extends MikanVariantBase {
  value: MikanVector3f = new MikanVector3f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector3f' }
  ];
}

export class MikanVector4dValue extends MikanVariantBase {
  value: MikanVector4d = new MikanVector4d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector4d' }
  ];
}

export class MikanVector4fValue extends MikanVariantBase {
  value: MikanVector4f = new MikanVector4f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector4f' }
  ];
}

