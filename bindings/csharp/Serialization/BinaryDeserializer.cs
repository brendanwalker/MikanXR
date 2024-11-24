using Newtonsoft.Json.Linq;
using System;
using System.Collections;
using System.IO;
using System.Reflection;

namespace MikanXR
{
	public class BinaryDeserializer
	{
		private class BinaryReadVisitor : IVisitor
		{
			private BinaryReader _reader;

			public BinaryReadVisitor(BinaryReader reader)
			{
				_reader = reader;
			}

			public void visitClass(ValueAccessor accessor)
			{
				var objectValue = accessor.getValueObject();
				var objectType = accessor.ValueType;

				// Writing into the current container element
				Utils.visitObject(objectValue, objectType, this);
			}

			public void visitList(ValueAccessor accessor)
			{
				// Get the list to be deserialized
				var list = accessor.getValueObject() as IList;
				Type listType = accessor.ValueType;
				Type elementType = listType.GenericTypeArguments[0];

				// Read the size of the array
				Int32 arraySize = _reader.ReadInt32();

				// Serialize each element of the array
				list.Clear();
				for (Int32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
				{
					// Make a fake "field" for an element in the array
					var elementAccessor = new ValueAccessor(null, elementType);

					// Serialize the element into a json object
					Utils.visitValue(elementAccessor, this);

					// Add the element to the list
					list.Add(elementAccessor.ValueInstance);
				}
			}

			public void visitDictionary(ValueAccessor accessor)
			{
				// Get the map to be deserialized
				var map = accessor.getValueObject() as IDictionary;
				Type mapType = accessor.ValueType;
				Type keyType = mapType.GenericTypeArguments[0];
				Type valueType = mapType.GenericTypeArguments[1];

				// Read the size of the array
				Int32 arraySize = _reader.ReadInt32();

				// Serialize each element of the array
				map.Clear();
				for (Int32 pairIndex = 0; pairIndex < arraySize; pairIndex++)
				{
					// Deserialize the key
					var keyAccessor = new ValueAccessor(null, keyType);
					Utils.visitValue(keyAccessor, this);

					// Deserialize the value
					var valueAccessor = new ValueAccessor(null, valueType);
					Utils.visitValue(valueAccessor, this);

					// Add the element to the list
					map.Add(keyAccessor.ValueInstance, valueAccessor.ValueInstance);
				}
			}

			public void visitPolymorphicObject(ValueAccessor accessor)
			{
				// Get the SerializableObject container
				var serializableObject = accessor.getValueObject();
				var serializableObjectType = accessor.ValueType;

				// Read the runtime class info
				string instanceClassName = readUTF8String();
				_ = _reader.ReadInt64();

				bool isValidObject = _reader.ReadBoolean();
				if (isValidObject)
				{
					// Allocate instance by class name
					var instance = 
						Utils.allocateMikanTypeByName(
							instanceClassName, out Type instanceType);

					// Deserialize the instance
					Utils.visitObject(instance, instanceType, this);

					// Assign the child object to the SerializableObject field
					MethodInfo setInstanceMethod = serializableObjectType.GetMethod("setInstance");
					setInstanceMethod.Invoke(serializableObject, new object[] { instance });
				}
			}

			public void visitEnum(ValueAccessor accessor)
			{
				string enumStringValue = readUTF8String();
				var enumValue= Enum.Parse(accessor.ValueType, enumStringValue);

				accessor.setValue(enumValue);
			}

			public void visitBool(ValueAccessor accessor)
			{
				bool value = _reader.ReadBoolean();

				accessor.setValue(value);
			}

			public void visitByte(ValueAccessor accessor)
			{
				sbyte value = _reader.ReadSByte();

				accessor.setValue(value);
			}

			public void visitUByte(ValueAccessor accessor)
			{
				byte value = _reader.ReadByte();

				accessor.setValue(value);
			}

			public void visitShort(ValueAccessor accessor)
			{
				Int16 value = _reader.ReadInt16();

				accessor.setValue(value);
			}

			public void visitUShort(ValueAccessor accessor)
			{
				UInt16 value = _reader.ReadUInt16();

				accessor.setValue(value);
			}

			public void visitInt(ValueAccessor accessor)
			{
				Int32 value = _reader.ReadInt32();

				accessor.setValue(value);
			}

			public void visitUInt(ValueAccessor accessor)
			{
				UInt32 value = _reader.ReadUInt32();

				accessor.setValue(value);
			}

			public void visitLong(ValueAccessor accessor)
			{
				Int64 value = _reader.ReadInt64();

				accessor.setValue(value);
			}

			public void visitULong(ValueAccessor accessor)
			{
				UInt64 value = _reader.ReadUInt64();

				accessor.setValue(value);
			}

			public void visitFloat(ValueAccessor accessor)
			{
				float value = _reader.ReadSingle();

				accessor.setValue(value);
			}

			public void visitDouble(ValueAccessor accessor)
			{
				double value = _reader.ReadDouble();

				accessor.setValue(value);
			}

			public void visitString(ValueAccessor accessor)
			{
				string value = readUTF8String();

				accessor.setValue(value);
			}

			protected string readUTF8String()
			{
				Int32 byteCount= _reader.ReadInt32();
				byte[] bytes= _reader.ReadBytes(byteCount);

				return System.Text.Encoding.UTF8.GetString(bytes);
			}
		}

		public static bool DeserializeFromBytes<T>(byte[] inBytes, T instance) where T : struct
		{
			return DeserializeFromBytes(inBytes, instance, typeof(T));
		}

		public static bool DeserializeFromBytes(
			byte[] inBytes,
			object instance,
			Type structType)
		{
			try
			{
				var binaryReader = new BinaryReader(new MemoryStream(inBytes));
				var visitor = new BinaryReadVisitor(binaryReader);
				Utils.visitObject(instance, structType, visitor);

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine("Failed to deserialize from JSON string: " + e.Message);
				return false;
			}
		}
	}
}
