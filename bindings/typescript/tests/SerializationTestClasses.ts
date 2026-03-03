/**
 * Test classes for serialization unit tests
 * Mirrors the C# test classes in SerializationUnitTests.cs
 */

import { PolymorphicStruct, PolymorphicObject } from '../PolymorphicObject';
import { FieldMetadata } from '../Serialization/SerializationUtils';

export enum SerializationTestEnum {
  Value1 = 0,
  Value2 = 1,
  Value3 = 2
}

export class SerializationPoint extends PolymorphicStruct {
  static readonly classId: bigint = 0n;
}

export class SerializationPoint2d extends SerializationPoint {
  static readonly classId: bigint = 1n;

  x_field: number = 0;
  y_field: number = 0;

  constructor(x: number = 0, y: number = 0) {
    super();
    this.x_field = x;
    this.y_field = y;
  }

  // Serialization metadata
  static __serializationMetadata: FieldMetadata[] = [
    { name: 'x_field', type: 'float' },
    { name: 'y_field', type: 'float' }
  ];
}

export class SerializationPoint3d extends SerializationPoint {
  static readonly classId: bigint = 2n;

  x_field: number = 0;
  y_field: number = 0;
  z_field: number = 0;

  constructor(x: number = 0, y: number = 0, z: number = 0) {
    super();
    this.x_field = x;
    this.y_field = y;
    this.z_field = z;
  }

  // Serialization metadata
  static __serializationMetadata: FieldMetadata[] = [
    { name: 'x_field', type: 'float' },
    { name: 'y_field', type: 'float' },
    { name: 'z_field', type: 'float' }
  ];
}

export class SerializationTestObject {
  bool_field: boolean = false;
  byte_field: number = 0;
  ubyte_field: number = 0;
  short_field: number = 0;
  ushort_field: number = 0;
  int_field: number = 0;
  uint_field: number = 0;
  long_field: bigint = 0n;
  float_field: number = 0;
  double_field: number = 0;
  string_field: string = '';
  enum_field: SerializationTestEnum = SerializationTestEnum.Value1;
  point2d_field: SerializationPoint2d = new SerializationPoint2d();
  point_ptr_field: PolymorphicObject = new PolymorphicObject();
  null_ptr_field: PolymorphicObject = new PolymorphicObject();
  bool_array: boolean[] = [];
  int_array: number[] = [];
  point2d_array: SerializationPoint2d[] = [];
  int_point_map: Map<number, SerializationPoint2d> = new Map();
  string_point_map: Map<string, SerializationPoint2d> = new Map();

  // Serialization metadata
  static __serializationMetadata: FieldMetadata[] = [
    { name: 'bool_field', type: 'boolean' },
    { name: 'byte_field', type: 'int8' },
    { name: 'ubyte_field', type: 'uint8' },
    { name: 'short_field', type: 'int16' },
    { name: 'ushort_field', type: 'uint16' },
    { name: 'int_field', type: 'int32' },
    { name: 'uint_field', type: 'uint32' },
    { name: 'long_field', type: 'bigint' },
    { name: 'float_field', type: 'float' },
    { name: 'double_field', type: 'double' },
    { name: 'string_field', type: 'string' },
    { name: 'enum_field', type: 'enum:SerializationTestEnum' },
    { name: 'point2d_field', type: 'SerializationPoint2d' },
    { name: 'point_ptr_field', type: 'PolymorphicObject' },
    { name: 'null_ptr_field', type: 'PolymorphicObject' },
    { name: 'bool_array', type: 'boolean', isArray: true },
    { name: 'int_array', type: 'int32', isArray: true },
    { name: 'point2d_array', type: 'SerializationPoint2d', isArray: true },
    {
      name: 'int_point_map',
      type: 'Map',
      isMap: true,
      keyType: 'int32',
      valueType: 'SerializationPoint2d'
    },
    {
      name: 'string_point_map',
      type: 'Map',
      isMap: true,
      keyType: 'string',
      valueType: 'SerializationPoint2d'
    }
  ];
}

/**
 * Build a test object with sample data
 */
export function buildSerializationTestObject(): SerializationTestObject {
  const testObject = new SerializationTestObject();

  const boolArray = [true, false, true];
  const intArray = [1, 2, 3];

  const pointArray = [
    new SerializationPoint2d(1.2345, 5.4321),
    new SerializationPoint2d(5.4321, 1.2345)
  ];

  const intPointMap = new Map<number, SerializationPoint2d>();
  intPointMap.set(1, new SerializationPoint2d(1.2345, 5.4321));
  intPointMap.set(2, new SerializationPoint2d(5.4321, 1.2345));

  const stringPointMap = new Map<string, SerializationPoint2d>();
  stringPointMap.set('key1', new SerializationPoint2d(1.2345, 5.4321));
  stringPointMap.set('key2', new SerializationPoint2d(5.4321, 1.2345));

  testObject.bool_field = true;
  testObject.byte_field = -123;
  testObject.ubyte_field = 123;
  testObject.short_field = -1234;
  testObject.ushort_field = 1234;
  testObject.int_field = -123456;
  testObject.uint_field = 123456;
  testObject.long_field = -123456789n;
  testObject.float_field = 1.2345;
  testObject.double_field = 1.23456789;
  testObject.string_field = 'hello';
  testObject.enum_field = SerializationTestEnum.Value2;
  testObject.point2d_field = new SerializationPoint2d(1.2345, 5.4321);
  testObject.point_ptr_field = new PolymorphicObject();
  testObject.point_ptr_field.setInstance(
    new SerializationPoint3d(1.2345, 5.4321, 9.8765),
    SerializationPoint3d.classId,
    'SerializationPoint3d'
  );
  testObject.null_ptr_field = new PolymorphicObject();
  testObject.bool_array = boolArray;
  testObject.int_array = intArray;
  testObject.point2d_array = pointArray;
  testObject.int_point_map = intPointMap;
  testObject.string_point_map = stringPointMap;

  return testObject;
}

/**
 * Verify that two test objects are equal
 */
export function verifySerializationTestObject(
  actual: SerializationTestObject,
  expected: SerializationTestObject
): { success: boolean; message?: string } {
  // Use a larger epsilon for float32 precision (approximately 1e-6)
  // Float32 has ~7 decimal digits of precision
  const floatEpsilon = 1e-6;

  // Helper for float comparison
  const floatEqual = (a: number, b: number): boolean => {
    return Math.abs(a - b) <= floatEpsilon;
  };

  // Check primitive fields
  if (actual.bool_field !== expected.bool_field) {
    return { success: false, message: 'bool_field mismatch' };
  }

  if (actual.byte_field !== expected.byte_field) {
    return { success: false, message: 'byte_field mismatch' };
  }

  if (actual.ubyte_field !== expected.ubyte_field) {
    return { success: false, message: 'ubyte_field mismatch' };
  }

  if (actual.short_field !== expected.short_field) {
    return { success: false, message: 'short_field mismatch' };
  }

  if (actual.ushort_field !== expected.ushort_field) {
    return { success: false, message: 'ushort_field mismatch' };
  }

  if (actual.int_field !== expected.int_field) {
    return { success: false, message: 'int_field mismatch' };
  }

  if (actual.uint_field !== expected.uint_field) {
    return { success: false, message: 'uint_field mismatch' };
  }

  if (actual.long_field !== expected.long_field) {
    return { success: false, message: 'long_field mismatch' };
  }

  if (!floatEqual(actual.float_field, expected.float_field)) {
    return { success: false, message: 'float_field mismatch' };
  }

  if (!floatEqual(actual.double_field, expected.double_field)) {
    return { success: false, message: 'double_field mismatch' };
  }

  if (actual.string_field !== expected.string_field) {
    return { success: false, message: 'string_field mismatch' };
  }

  if (actual.enum_field !== expected.enum_field) {
    return { success: false, message: 'enum_field mismatch' };
  }

  // Check point2d_field
  if (!floatEqual(actual.point2d_field.x_field, expected.point2d_field.x_field)) {
    return { success: false, message: 'point2d_field.x_field mismatch' };
  }

  if (!floatEqual(actual.point2d_field.y_field, expected.point2d_field.y_field)) {
    return { success: false, message: 'point2d_field.y_field mismatch' };
  }

  // Check point_ptr_field (polymorphic object)
  const expectedPoint3d = expected.point_ptr_field.instance as SerializationPoint3d;
  const actualPoint3d = actual.point_ptr_field.instance as SerializationPoint3d;

  if (!expectedPoint3d || !actualPoint3d) {
    return { success: false, message: 'point_ptr_field instance is null' };
  }

  if (!floatEqual(expectedPoint3d.x_field, actualPoint3d.x_field)) {
    return { success: false, message: 'point_ptr_field.x_field mismatch' };
  }

  if (!floatEqual(expectedPoint3d.y_field, actualPoint3d.y_field)) {
    return { success: false, message: 'point_ptr_field.y_field mismatch' };
  }

  if (!floatEqual(expectedPoint3d.z_field, actualPoint3d.z_field)) {
    return { success: false, message: 'point_ptr_field.z_field mismatch' };
  }

  // Check null_ptr_field
  if (expected.null_ptr_field.instance !== null) {
    return { success: false, message: 'expected.null_ptr_field.instance should be null' };
  }

  if (expected.null_ptr_field.runtimeClassId !== 0n) {
    return { success: false, message: 'expected.null_ptr_field.runtimeClassId should be 0' };
  }

  if (actual.null_ptr_field.instance !== null) {
    return { success: false, message: 'actual.null_ptr_field.instance should be null' };
  }

  if (actual.null_ptr_field.runtimeClassId !== 0n) {
    return { success: false, message: 'actual.null_ptr_field.runtimeClassId should be 0' };
  }

  // Check bool_array
  if (actual.bool_array.length !== expected.bool_array.length) {
    return { success: false, message: 'bool_array length mismatch' };
  }

  for (let i = 0; i < actual.bool_array.length; i++) {
    if (actual.bool_array[i] !== expected.bool_array[i]) {
      return { success: false, message: `bool_array[${i}] mismatch` };
    }
  }

  // Check int_array
  if (actual.int_array.length !== expected.int_array.length) {
    return { success: false, message: 'int_array length mismatch' };
  }

  for (let i = 0; i < actual.int_array.length; i++) {
    if (actual.int_array[i] !== expected.int_array[i]) {
      return { success: false, message: `int_array[${i}] mismatch` };
    }
  }

  // Check point2d_array
  if (actual.point2d_array.length !== expected.point2d_array.length) {
    return { success: false, message: 'point2d_array length mismatch' };
  }

  for (let i = 0; i < actual.point2d_array.length; i++) {
    const actualPoint = actual.point2d_array[i];
    const expectedPoint = expected.point2d_array[i];

    if (!floatEqual(actualPoint.x_field, expectedPoint.x_field)) {
      return { success: false, message: `point2d_array[${i}].x_field mismatch` };
    }

    if (!floatEqual(actualPoint.y_field, expectedPoint.y_field)) {
      return { success: false, message: `point2d_array[${i}].y_field mismatch` };
    }
  }

  // Check int_point_map
  if (actual.int_point_map.size !== expected.int_point_map.size) {
    return { success: false, message: 'int_point_map size mismatch' };
  }

  for (const [key, actualPoint] of actual.int_point_map) {
    const expectedPoint = expected.int_point_map.get(key);
    if (!expectedPoint) {
      return { success: false, message: `int_point_map missing key ${key}` };
    }

    if (!floatEqual(actualPoint.x_field, expectedPoint.x_field)) {
      return { success: false, message: `int_point_map[${key}].x_field mismatch` };
    }

    if (!floatEqual(actualPoint.y_field, expectedPoint.y_field)) {
      return { success: false, message: `int_point_map[${key}].y_field mismatch` };
    }
  }

  // Check string_point_map
  if (actual.string_point_map.size !== expected.string_point_map.size) {
    return { success: false, message: 'string_point_map size mismatch' };
  }

  for (const [key, actualPoint] of actual.string_point_map) {
    const expectedPoint = expected.string_point_map.get(key);
    if (!expectedPoint) {
      return { success: false, message: `string_point_map missing key ${key}` };
    }

    if (!floatEqual(actualPoint.x_field, expectedPoint.x_field)) {
      return { success: false, message: `string_point_map[${key}].x_field mismatch` };
    }

    if (!floatEqual(actualPoint.y_field, expectedPoint.y_field)) {
      return { success: false, message: `string_point_map[${key}].y_field mismatch` };
    }
  }

  return { success: true };
}
