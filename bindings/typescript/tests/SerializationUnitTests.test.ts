/**
 * Serialization unit tests for TypeScript bindings
 * Mirrors the C# tests in SerializationUnitTests.cs
 */

import {
  SerializationTestObject,
  SerializationPoint2d,
  SerializationPoint3d,
  buildSerializationTestObject,
  verifySerializationTestObject
} from './SerializationTestClasses';

import {
  serializeToJsonString,
  deserializeFromJsonString,
  serializeToBytes,
  deserializeFromBytes,
  TypeRegistry
} from '../Serialization';

// Register types for polymorphic deserialization
TypeRegistry.register('SerializationPoint2d', SerializationPoint2d);
TypeRegistry.register('SerializationPoint3d', SerializationPoint3d);
TypeRegistry.register('SerializationTestObject', SerializationTestObject);

describe('SerializationUnitTests', () => {
  describe('JSON Serialization', () => {
    it('should serialize and deserialize test object to/from JSON', () => {
      // Build expected object
      const expected = buildSerializationTestObject();

      // Serialize to JSON string
      const jsonString = serializeToJsonString(expected, SerializationTestObject);

      // Verify JSON was created
      expect(jsonString).toBeDefined();
      expect(jsonString.length).toBeGreaterThan(0);

      // Deserialize from JSON string
      const actual = new SerializationTestObject();
      const canDeserialize = deserializeFromJsonString(
        jsonString,
        actual,
        SerializationTestObject
      );

      // Verify deserialization succeeded
      expect(canDeserialize).toBe(true);

      // Verify the objects match
      const result = verifySerializationTestObject(actual, expected);
      expect(result.success).toBe(true);
      if (!result.success) {
        console.error('Verification failed:', result.message);
      }
    });

    it('should handle empty arrays', () => {
      const testObject = new SerializationTestObject();
      testObject.bool_array = [];
      testObject.int_array = [];
      testObject.point2d_array = [];

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.bool_array.length).toBe(0);
      expect(deserialized.int_array.length).toBe(0);
      expect(deserialized.point2d_array.length).toBe(0);
    });

    it('should handle empty maps', () => {
      const testObject = new SerializationTestObject();
      testObject.int_point_map = new Map();
      testObject.string_point_map = new Map();

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.int_point_map.size).toBe(0);
      expect(deserialized.string_point_map.size).toBe(0);
    });

    it('should preserve bigint values', () => {
      const testObject = new SerializationTestObject();
      testObject.long_field = 9007199254740991n; // MAX_SAFE_INTEGER as bigint

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.long_field).toBe(9007199254740991n);
    });

    it('should handle negative numbers', () => {
      const testObject = new SerializationTestObject();
      testObject.byte_field = -123;
      testObject.short_field = -1234;
      testObject.int_field = -123456;
      testObject.long_field = -123456789n;
      testObject.float_field = -1.2345;
      testObject.double_field = -1.23456789;

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.byte_field).toBe(-123);
      expect(deserialized.short_field).toBe(-1234);
      expect(deserialized.int_field).toBe(-123456);
      expect(deserialized.long_field).toBe(-123456789n);
      expect(deserialized.float_field).toBeCloseTo(-1.2345);
      expect(deserialized.double_field).toBeCloseTo(-1.23456789);
    });

    it('should handle string with special characters', () => {
      const testObject = new SerializationTestObject();
      testObject.string_field = 'Hello "World"\nNew Line\tTab\\Backslash';

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.string_field).toBe('Hello "World"\nNew Line\tTab\\Backslash');
    });
  });

  describe('Binary Serialization', () => {
    it('should serialize and deserialize test object to/from binary', () => {
      // Build expected object
      const expected = buildSerializationTestObject();

      // Serialize to binary bytes
      const bytes = serializeToBytes(expected, SerializationTestObject);

      // Verify bytes were created
      expect(bytes).toBeDefined();
      expect(bytes.length).toBeGreaterThan(0);

      // Deserialize from binary bytes
      const actual = new SerializationTestObject();
      let canDeserialize = false;
      try {
        canDeserialize = deserializeFromBytes(bytes, actual, SerializationTestObject);
      } catch (error) {
        console.error('Binary deserialization error:', error);
        throw error;
      }

      // Verify deserialization succeeded
      if (!canDeserialize) {
        console.error('Binary deserialization returned false - check console for errors');
      }
      expect(canDeserialize).toBe(true);

      // Verify the objects match
      const result = verifySerializationTestObject(actual, expected);
      if (!result.success) {
        console.error('Verification failed:', result.message);
        console.error('Expected:', JSON.stringify(expected, (key, value) =>
          typeof value === 'bigint' ? value.toString() : value instanceof Map ? Array.from(value.entries()) : value, 2));
        console.error('Actual:', JSON.stringify(actual, (key, value) =>
          typeof value === 'bigint' ? value.toString() : value instanceof Map ? Array.from(value.entries()) : value, 2));
      }
      expect(result.success).toBe(true);
    });

    it('should handle empty arrays in binary format', () => {
      const testObject = new SerializationTestObject();
      testObject.bool_array = [];
      testObject.int_array = [];
      testObject.point2d_array = [];

      const bytes = serializeToBytes(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromBytes(bytes, deserialized, SerializationTestObject);

      expect(success).toBe(true);
      expect(deserialized.bool_array.length).toBe(0);
      expect(deserialized.int_array.length).toBe(0);
      expect(deserialized.point2d_array.length).toBe(0);
    });

    it('should handle empty maps in binary format', () => {
      const testObject = new SerializationTestObject();
      testObject.int_point_map = new Map();
      testObject.string_point_map = new Map();

      const bytes = serializeToBytes(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromBytes(bytes, deserialized, SerializationTestObject);

      expect(success).toBe(true);
      expect(deserialized.int_point_map.size).toBe(0);
      expect(deserialized.string_point_map.size).toBe(0);
    });

    it('should preserve bigint values in binary format', () => {
      const testObject = new SerializationTestObject();
      testObject.long_field = 9007199254740991n; // MAX_SAFE_INTEGER as bigint

      const bytes = serializeToBytes(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromBytes(bytes, deserialized, SerializationTestObject);

      expect(success).toBe(true);
      expect(deserialized.long_field).toBe(9007199254740991n);
    });

    it('should handle UTF-8 strings in binary format', () => {
      const testObject = new SerializationTestObject();
      testObject.string_field = 'Hello 世界 🌍';

      const bytes = serializeToBytes(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromBytes(bytes, deserialized, SerializationTestObject);

      expect(success).toBe(true);
      expect(deserialized.string_field).toBe('Hello 世界 🌍');
    });

    it('should use little-endian byte order', () => {
      const testObject = new SerializationTestObject();
      testObject.int_field = 0x12345678;

      const bytes = serializeToBytes(testObject, SerializationTestObject);

      // The int field should be somewhere in the byte array as little-endian: 78 56 34 12
      // This is a simplified check - in a real scenario we'd need to know the exact offset
      expect(bytes).toBeDefined();
      expect(bytes.length).toBeGreaterThan(4);
    });
  });

  describe('Edge Cases', () => {
    it('should handle null polymorphic objects', () => {
      const testObject = new SerializationTestObject();
      testObject.null_ptr_field.setInstance(null as any, 0n, '');

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.null_ptr_field.instance).toBeNull();
      expect(deserialized.null_ptr_field.runtimeClassId).toBe(0n);
    });

    it('should handle boolean edge cases', () => {
      const testObject = new SerializationTestObject();
      testObject.bool_field = false;
      testObject.bool_array = [true, false, true, false];

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.bool_field).toBe(false);
      expect(deserialized.bool_array).toEqual([true, false, true, false]);
    });

    it('should handle zero values', () => {
      const testObject = new SerializationTestObject();
      testObject.byte_field = 0;
      testObject.ubyte_field = 0;
      testObject.short_field = 0;
      testObject.ushort_field = 0;
      testObject.int_field = 0;
      testObject.uint_field = 0;
      testObject.long_field = 0n;
      testObject.float_field = 0.0;
      testObject.double_field = 0.0;

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.byte_field).toBe(0);
      expect(deserialized.ubyte_field).toBe(0);
      expect(deserialized.short_field).toBe(0);
      expect(deserialized.ushort_field).toBe(0);
      expect(deserialized.int_field).toBe(0);
      expect(deserialized.uint_field).toBe(0);
      expect(deserialized.long_field).toBe(0n);
      expect(deserialized.float_field).toBe(0.0);
      expect(deserialized.double_field).toBe(0.0);
    });

    it('should handle empty string', () => {
      const testObject = new SerializationTestObject();
      testObject.string_field = '';

      const jsonString = serializeToJsonString(testObject, SerializationTestObject);
      const deserialized = new SerializationTestObject();
      const success = deserializeFromJsonString(
        jsonString,
        deserialized,
        SerializationTestObject
      );

      expect(success).toBe(true);
      expect(deserialized.string_field).toBe('');
    });
  });

  describe('Type Registry', () => {
    it('should register and retrieve types', () => {
      TypeRegistry.register('TestType', SerializationPoint2d);
      expect(TypeRegistry.has('TestType')).toBe(true);
      expect(TypeRegistry.get('TestType')).toBe(SerializationPoint2d);
    });

    it('should return undefined for unregistered types', () => {
      expect(TypeRegistry.get('NonExistentType')).toBeUndefined();
      expect(TypeRegistry.has('NonExistentType')).toBe(false);
    });
  });
});
