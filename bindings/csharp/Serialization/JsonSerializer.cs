﻿using Newtonsoft.Json.Linq;
using System;
using System.Collections;

namespace MikanXR
{
	public class JsonSerializer
	{
		private class JsonWriteVisitor : IVisitor
		{
			private JObject _jsonObject;

			public JsonWriteVisitor(JObject jsonObject)
			{
				_jsonObject = jsonObject;
			}

			public void visitClass(ValueAccessor accessor)
			{
				var objectValue = accessor.getValueObject();
				var objectType = accessor.ValueType;

				var objectOwnerField = accessor.ValueField;
				if (objectOwnerField != null)
				{
					// Writing into a named field
					var jsonObject = new JObject();
					var jsonVisitor = new JsonWriteVisitor(jsonObject);
					Utils.visitObject(objectValue, objectType, jsonVisitor);

					_jsonObject[objectOwnerField.Name] = jsonObject;
				}
				else
				{
					// Writing into the current container element
					Utils.visitObject(objectValue, objectType, this);
				}
			}

			public void visitList(ValueAccessor accessor)
			{
				// Get the list to be serialized
				var list = accessor.getValueObject() as IList;
				Type listType = accessor.ValueType;
				Type elementType = listType.GenericTypeArguments[0];

				// Serialize each element of the array
				JArray jsonArray = new JArray();
				foreach (var element in list)
				{
					// Make a fake "field" for an element in the array
					var elementAccessor = new ValueAccessor(element, elementType);

					// Serialize the element into a json object
					var jsonElement = new JObject();
					var elementVisitor = new JsonWriteVisitor(jsonElement);
					Utils.visitValue(elementAccessor, elementVisitor);

					// Add the element to the json array
					jsonArray.Add(elementAccessor);
				}

				var arrayOwnerField = accessor.ValueField;
				if (arrayOwnerField != null)
				{
					// Writing into a named field
					_jsonObject[accessor.ValueName] = jsonArray;
				}
				else
				{
					// Writing into the current container element
					_jsonObject.Add(jsonArray);
				}
			}

			public void visitDictionary(ValueAccessor accessor)
			{
				// Get the map to be serialized
				var map = accessor.getValueObject() as IDictionary;
				Type mapType = accessor.ValueType;
				Type keyType = mapType.GenericTypeArguments[0];
				Type valueType = mapType.GenericTypeArguments[1];

				// Serialize each key-value pair in the map
				JArray jsonPairArray = new JArray();
				IDictionaryEnumerator mapEnumerator = map.GetEnumerator();
				while (mapEnumerator.MoveNext())
				{
					// Serialize the key into a json object
					var keyObject = mapEnumerator.Key;
					var jsonKey = new JObject();
					var keyVisitor = new JsonWriteVisitor(jsonKey);
					var keyAccessor = new ValueAccessor(keyObject, keyType);
					Utils.visitValue(keyAccessor, keyVisitor);

					// Serialize the value into a json object
					var valueObject = mapEnumerator.Value;
					var jsonValue = new JObject();
					var valueVisitor = new JsonWriteVisitor(jsonValue);
					var valueAccessor = new ValueAccessor(valueObject, valueType);
					Utils.visitValue(valueAccessor, valueVisitor);

					var jsonPair = new JObject();
					jsonPair["key"]= jsonKey;
					jsonPair["value"]= jsonValue;

					// Add the pairs to the json array
					jsonPairArray.Add(jsonPair);
				}

				var mapOwnerField = accessor.ValueField;
				if (mapOwnerField != null)
				{
					// Writing into a named field
					_jsonObject[accessor.ValueName] = jsonPairArray;
				}
				else
				{
					// Writing into the current container element
					_jsonObject.Add(jsonPairArray);
				}
			}

			public void visitPolymorphicObject(ValueAccessor accessor)
			{
				// Get the SerializableObject container
				var serializableObject = accessor.getValueObject();
				var serializableObjectType = accessor.ValueType;

				// Get the instance and runtime class properties
				var instanceClassIdProperty = serializableObjectType.GetProperty("RuntimeClassId");
				var instanceProperty = serializableObjectType.GetProperty("Instance");

				var instanceClassId = (ulong)instanceClassIdProperty.GetValue(serializableObject);
				var instance = instanceProperty.GetValue(instanceProperty);
				Type instanceType = instance.GetType();
				var instanceClassName = instanceType.Name;

				// Serialize the object into a json object
				var jsonRuntimeObject = new JObject();
				var jsonVisitor = new JsonWriteVisitor(jsonRuntimeObject);
				Utils.visitObject(instance, instanceType, jsonVisitor);

				var jsonSerializableObject = new JObject();
				jsonSerializableObject["class_name"] = instanceClassName;
				jsonSerializableObject["class_id"] = instanceClassId;
				jsonSerializableObject["value"] = jsonRuntimeObject;

				var objectOwnerField = accessor.ValueField;
				if (objectOwnerField != null)
				{
					// Writing into a named field
					_jsonObject[objectOwnerField.Name] = jsonSerializableObject;
				}
				else
				{
					// Writing into the current container element
					Utils.visitObject(jsonSerializableObject, serializableObjectType, this);
				}
			}

			public void visitEnum(ValueAccessor accessor)
			{
				object enumObjectValue = accessor.getValueObject();
				string enumStringValue = Enum.GetName(accessor.ValueType, enumObjectValue);

				var field = accessor.ValueField;
				if (field != null)
				{
					_jsonObject.Add(field.Name, new JValue(enumStringValue));
				}
				else
				{
					_jsonObject.Add(enumStringValue);
				}
			}

			public void visitBool(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<bool>(accessor);
			}

			public void visitByte(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<sbyte>(accessor);
			}

			public void visitUByte(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<byte>(accessor);
			}

			public void visitShort(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<short>(accessor);
			}

			public void visitUShort(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<ushort>(accessor);
			}

			public void visitInt(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<uint>(accessor);
			}

			public void visitUInt(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<uint>(accessor);
			}

			public void visitLong(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<long>(accessor);
			}

			public void visitULong(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<ulong>(accessor);
			}

			public void visitFloat(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<float>(accessor);
			}

			public void visitDouble(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<double>(accessor);
			}
			public void visitString(ValueAccessor accessor)
			{
				setJsonValueFromAccessor<string>(accessor);
			}

			private void setJsonValueFromAccessor<T>(ValueAccessor accessor)
			{
				var field= accessor.ValueField;
				T value = accessor.getValue<T>();

				if (field != null) 
				{
					_jsonObject.Add(field.Name, new JValue(value));
				}
				else
				{
					_jsonObject.Add(value);
				}
			}
		}

		public static string serializeToJsonString<T>(T instance) where T : class
		{
			return serializeToJsonString(instance, typeof(T));
		}

		public static string serializeToJsonString(object instance, Type structType)
		{
			JObject json = serializeToJson(instance, structType);
			return json.ToString();
		}

		public static JObject serializeToJson<T>(T instance) where T : struct
		{
			return serializeToJson(instance, typeof(T));
		}

		public static JObject serializeToJson(object instance, Type structType)
		{
			var jsonObject = new JObject();
			var visitor = new JsonWriteVisitor(jsonObject);

			Utils.visitObject(instance, structType, visitor);

			return jsonObject;
		}
	}
}