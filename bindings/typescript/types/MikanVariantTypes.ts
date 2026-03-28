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

export class MikanVariantBase extends PolymorphicStruct {

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
  ];
}

export class MikanVector2fValue extends MikanVariantBase {
  value: MikanVector2f = new MikanVector2f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector2f' }
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

export class MikanIntArrayValue extends MikanVariantBase {
  value: number[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'int32', isArray: true }
  ];
}

export class MikanMatrix4fValue extends MikanVariantBase {
  value: MikanMatrix4f = new MikanMatrix4f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanMatrix4f' }
  ];
}

export class MikanBoolValue extends MikanVariantBase {
  value: boolean = false;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'boolean' }
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

export class MikanFloatValue extends MikanVariantBase {
  value: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'float' }
  ];
}

export class MikanDoubleValue extends MikanVariantBase {
  value: number = 0;

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'double' }
  ];
}

export class MikanVector3fValue extends MikanVariantBase {
  value: MikanVector3f = new MikanVector3f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector3f' }
  ];
}

export class MikanVector4fValue extends MikanVariantBase {
  value: MikanVector4f = new MikanVector4f();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector4f' }
  ];
}

export class MikanQuatfValue extends MikanVariantBase {
  value: MikanQuatf = new MikanQuatf();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanQuatf' }
  ];
}

export class MikanVector2dValue extends MikanVariantBase {
  value: MikanVector2d = new MikanVector2d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector2d' }
  ];
}

export class MikanVector3dValue extends MikanVariantBase {
  value: MikanVector3d = new MikanVector3d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector3d' }
  ];
}

export class MikanVector4dValue extends MikanVariantBase {
  value: MikanVector4d = new MikanVector4d();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanVector4d' }
  ];
}

export class MikanQuatdValue extends MikanVariantBase {
  value: MikanQuatd = new MikanQuatd();

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'MikanQuatd' }
  ];
}

export class MikanBoolArrayValue extends MikanVariantBase {
  value: boolean[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'boolean' }
  ];
}

export class MikanFloatArrayValue extends MikanVariantBase {
  value: number[] = [];

  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [
    { name: 'value', type: 'float', isArray: true }
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

