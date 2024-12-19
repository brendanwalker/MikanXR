using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Linq;

namespace MikanXR
{
	enum SerializationTestEnum
	{
		Value1,
		Value2,
		Value3,
	};

	class SerializationPoint
	{
		public static readonly ulong classId= 0;
	};

	class SerializationPoint2d : SerializationPoint
	{
		public static new readonly ulong classId= 1;

		public SerializationPoint2d()
		{
		}

		public SerializationPoint2d(float x, float y)
		{
			x_field = x;
			y_field = y;
		}

		public float x_field = 0;
		public float y_field = 0;
	};

	class SerializationPoint3d : SerializationPoint
	{
		public static new readonly ulong classId= 2;

		public SerializationPoint3d()
		{
		}

		public SerializationPoint3d(float x, float y, float z)
		{
			x_field = x;
			y_field = y;
			z_field = z;
		}

		public float x_field = 0;
		public float y_field = 0;
		public float z_field = 0;
	};


	class SerializationTestObject
	{
		public bool bool_field;
		public sbyte byte_field;
		public byte ubyte_field;
		public short short_field;
		public ushort ushort_field;
		public int int_field;
		public uint uint_field;
		public long long_field;
		public ulong ulong_field;
		public float float_field;
		public double double_field;
		public string string_field;
		public SerializationTestEnum enum_field;
		public SerializationPoint2d point2d_field;
		public SerializableObject<SerializationPoint> point_ptr_field;
		public List<bool> bool_array;
		public List<int> int_array;
		public List<SerializationPoint2d> point2d_array;
		public Dictionary<int, SerializationPoint2d> int_point_map;
		public Dictionary<string, SerializationPoint2d> string_point_map;
	};

	[TestFixture]
	public class SerializationUnitTests
	{
		SerializationTestObject buildSerializationTestObject()
		{
			SerializationTestObject testObject = new SerializationTestObject();
			var boolArray = new List<bool>() { true, false, true };
			var intArray = new List<int>() {1, 2, 3};

			var pointArray = new List<SerializationPoint2d>();
			pointArray.Add( new SerializationPoint2d(1.2345f, 5.4321f) );
			pointArray.Add( new SerializationPoint2d(5.4321f, 1.2345f) );

			var intPointMap = new Dictionary<int, SerializationPoint2d>();
			intPointMap.Add(1, new SerializationPoint2d(1.2345f, 5.4321f));
			intPointMap.Add(2, new SerializationPoint2d(5.4321f, 1.2345f));

			var stringPointMap = new Dictionary<string, SerializationPoint2d>();
			stringPointMap.Add("key1", new SerializationPoint2d(1.2345f, 5.4321f));
			stringPointMap.Add("key2", new SerializationPoint2d(5.4321f, 1.2345f));

			testObject.bool_field= true;
			testObject.byte_field= -123;
			testObject.ubyte_field= 123;
			testObject.short_field= -1234;
			testObject.ushort_field= 1234;
			testObject.int_field= -123456;
			testObject.uint_field= 123456;
			testObject.long_field= -123456789;
			testObject.ulong_field= 123456789;
			testObject.float_field= 1.2345f;
			testObject.double_field= 1.23456789;
			testObject.string_field= "hello";
			testObject.enum_field= SerializationTestEnum.Value2;
			testObject.point2d_field= new SerializationPoint2d(1.2345f, 5.4321f);
			testObject.point_ptr_field = new SerializableObject<SerializationPoint>();
			testObject.point_ptr_field.setInstance(new SerializationPoint3d(1.2345f, 5.4321f, 9.8765f));
			testObject.bool_array= boolArray;
			testObject.int_array= intArray;
			testObject.point2d_array= pointArray;
			testObject.int_point_map= intPointMap;
			testObject.string_point_map= stringPointMap;

			return testObject;
		}

		void verifySerializationTestObject(
			SerializationTestObject actual, 
			SerializationTestObject expected)
		{
			Assert.That(actual.bool_field, Is.EqualTo(expected.bool_field));
			Assert.That(actual.byte_field, Is.EqualTo(expected.byte_field));
			Assert.That(actual.ubyte_field, Is.EqualTo(expected.ubyte_field));
			Assert.That(actual.short_field, Is.EqualTo(expected.short_field));
			Assert.That(actual.ushort_field, Is.EqualTo(expected.ushort_field));
			Assert.That(actual.int_field, Is.EqualTo(expected.int_field));
			Assert.That(actual.uint_field, Is.EqualTo(expected.uint_field));
			Assert.That(actual.long_field, Is.EqualTo(expected.long_field));
			Assert.That(actual.ulong_field, Is.EqualTo(expected.ulong_field));
			Assert.That(actual.float_field, Is.EqualTo(expected.float_field).Within(float.Epsilon));
			Assert.That(actual.double_field, Is.EqualTo(expected.double_field).Within(double.Epsilon));
			Assert.That(actual.string_field, Is.EqualTo(expected.string_field));
			Assert.That(actual.enum_field, Is.EqualTo(expected.enum_field));
			Assert.That(actual.point2d_field.x_field, Is.EqualTo(expected.point2d_field.x_field).Within(float.Epsilon));
			Assert.That(actual.point2d_field.y_field, Is.EqualTo(expected.point2d_field.y_field).Within(float.Epsilon));

			var expected_point3d= (SerializationPoint3d)expected.point_ptr_field.Instance;
			var actual_point3d= (SerializationPoint3d)actual.point_ptr_field.Instance;
			Assert.That(expected_point3d.x_field, Is.EqualTo(actual_point3d.x_field).Within(float.Epsilon));
			Assert.That(expected_point3d.y_field, Is.EqualTo(actual_point3d.y_field).Within(float.Epsilon));
			Assert.That(expected_point3d.z_field, Is.EqualTo(actual_point3d.z_field).Within(float.Epsilon));

			Assert.That(actual.bool_array.Count, Is.EqualTo(expected.bool_array.Count));
			for (int i = 0; i < actual.bool_array.Count; ++i)
			{
				Assert.That(actual.bool_array[i], Is.EqualTo(expected.bool_array[i]));
			}

			Assert.That(actual.int_array.Count, Is.EqualTo(expected.int_array.Count));
			for (int i = 0; i < actual.int_array.Count; ++i)
			{
				Assert.That(actual.int_array[i], Is.EqualTo(expected.int_array[i]));
			}

			Assert.That(actual.point2d_array.Count, Is.EqualTo(expected.point2d_array.Count));
			for (int i = 0; i < 2; ++i)
			{
				var actualPoint = actual.point2d_array[i];
				var expectedPoint = expected.point2d_array[i];

				Assert.That(actualPoint.x_field, Is.EqualTo(expectedPoint.x_field).Within(float.Epsilon));
				Assert.That(actualPoint.y_field, Is.EqualTo(expectedPoint.y_field).Within(float.Epsilon));
			}

			Assert.That(actual.int_point_map.Count, Is.EqualTo(expected.int_point_map.Count));
			foreach (var pair in actual.int_point_map)
			{
				var key = pair.Key;
				var actualPoint = pair.Value;
				var expectedPoint = expected.int_point_map[key];

				Assert.That(actualPoint.x_field, Is.EqualTo(expectedPoint.x_field).Within(float.Epsilon));
				Assert.That(actualPoint.y_field, Is.EqualTo(expectedPoint.y_field).Within(float.Epsilon));
			}

			Assert.That(actual.string_point_map.Count, Is.EqualTo(expected.string_point_map.Count));
			foreach (var pair in actual.string_point_map)
			{
				var key = pair.Key;
				var actualPoint = pair.Value;
				var expectedPoint = expected.string_point_map[key];

				Assert.That(actualPoint.x_field, Is.EqualTo(expectedPoint.x_field).Within(float.Epsilon));
				Assert.That(actualPoint.y_field, Is.EqualTo(expectedPoint.y_field).Within(float.Epsilon));
			}
		}

		[Test]
		public void TestReflectionFromJson()
		{
			// Arrange
			var expected = buildSerializationTestObject();

			// Act
			string jsonString = JsonSerializer.serializeToJsonString(expected);
			var actual = new SerializationTestObject();
			bool bCanDeserialize= JsonDeserializer.deserializeFromJsonString(jsonString, actual);

			// Assert
			Assert.That(bCanDeserialize);
			verifySerializationTestObject(actual, expected);
		}

		[Test]
		public void TestReflectionFromBytes()
		{
			// Arrange
			var expected = buildSerializationTestObject();

			// Act
			byte[] bytes = BinarySerializer.SerializeToBytes(expected);

			var actual = new SerializationTestObject();
			bool bCanDeserialize = BinaryDeserializer.DeserializeFromBytes(bytes, actual);

			// Assert
			Assert.That(bCanDeserialize);
			verifySerializationTestObject(actual, expected);
		}
	}
}