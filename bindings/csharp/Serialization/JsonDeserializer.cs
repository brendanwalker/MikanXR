using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Reflection;

namespace MikanXR
{
	public class JsonDeserializer
	{
		private class JsonReadVisitor : IVisitor
		{
			private JObject _jsonObject;

			public JsonReadVisitor(JObject jsonObject)
			{
				_jsonObject = jsonObject;
			}

			public void visitClass(ValueAccessor accessor)
			{
				JToken jsonToken = getJsonTokenFromAccessor(accessor);
				Type fieldType = accessor.ValueType;
				string fieldName= accessor.ValueName;
				object instance = accessor.ValueInstance;

				if (jsonToken.Type == JTokenType.Array)
				{

				}
				else if (jsonToken.Type == JTokenType.Object)
				{
					if (fieldType.IsGenericType)
					{
						if (fieldType.Name == "SerializableObject" &&
							fieldType.GenericTypeArguments.Length == 1)
						{
							visitObjectPtr(accessor, jsonToken as JObject);
						}
					}
					else
					{
						JsonReadVisitor jsonVisitor = new JsonReadVisitor(jsonToken as JObject);

						Utils.visitObject(instance, fieldType, jsonVisitor);
					}
				}
				else if (jsonToken.Type == JTokenType.String)
				{

				}
				else
				{
					throw new Exception(
						"JsonReadVisitor::visitStruct() Struct Field " + fieldName + 
						" was not of expected type object to deserialize json object value");
				}
			}

			public void visitEnum(ValueAccessor accessor)
			{
				//var stringEnumConverter = new Newtonsoft.Json.Converters.StringEnumConverter();
				//var settings = new JsonSerializerSettings();
				//settings.Converters.Add(stringEnumConverter);

				//T response = JsonConvert.DeserializeObject<T>(utfJsonString, settings);

			}

			public void visitBool(ValueAccessor accessor)
			{

			}

			public void visitByte(ValueAccessor accessor)
			{

			}

			public void visitUByte(ValueAccessor accessor)
			{

			}

			public void visitShort(ValueAccessor accessor)
			{

			}

			public void visitUShort(ValueAccessor accessor)
			{

			}

			public void visitInt(ValueAccessor accessor)
			{

			}

			public void visitUInt(ValueAccessor accessor)
			{

			}

			public void visitLong(ValueAccessor accessor)
			{

			}

			public void visitULong(ValueAccessor accessor)
			{

			}

			public void visitFloat(ValueAccessor accessor)
			{

			}

			public void visitDouble(ValueAccessor accessor)
			{

			}
			
			private JToken getJsonTokenFromAccessor(ValueAccessor accessor)
			{
				var field= accessor.ValueField;
				if (field != null)
				{
					var fieldName = field.Name;

					if (_jsonObject.TryGetValue(fieldName, out var childJsonObject))
					{
						return childJsonObject;
					}

					throw new Exception("Field "+fieldName+" not found in json");
				}
				else
				{
					// The json object should contain the value we want to deserialize
					return _jsonObject;
				}
			}

			private void visitObjectPtr(ValueAccessor accessor, JObject serializableObjectJson)
			{
				Type fieldType = accessor.ValueType;
				string fieldName = accessor.ValueName;
				object fieldInstance = accessor.ValueInstance;

				// Allocate the element instance using the runtime class name
				Type elementStaticType= fieldType.GenericTypeArguments[0];
				var elementRuntimeTypeName= (string)serializableObjectJson["class_name"];
				var elementInstance= 
					Utils.allocateMikanTypeByName(
						elementRuntimeTypeName, out Type elementRuntimeType);
				if (elementInstance == null)
				{
					throw new Exception(
							"JsonReadVisitor::visitObjectPtr() " +
							"ObjectPtr Accessor " + fieldName +
							" used an unknown runtime class_name: " + elementRuntimeTypeName +
							", static class_name: " + elementStaticType.Name);
				}

				// Deserialize the element into the newly allocated instance
				JObject elementJson = serializableObjectJson["value"] as JObject;
				var elementVisitor = new JsonReadVisitor(elementJson);
				Utils.visitObject(elementInstance, elementRuntimeType, elementVisitor);

				// Assign the child object to the SerializableObject field
				MethodInfo setInstanceMethod= fieldType.GetMethod("setInstance");
				setInstanceMethod.Invoke(fieldInstance, new object[] { elementInstance });
			}
		}

		public static bool deserializeFromJsonString<T>(string jsonString, T instance) where T : struct
		{
			return deserializeFromJsonString(jsonString, instance, typeof(T));
		}

		public static bool deserializeFromJsonString(
			string jsonString, 
			object instance, 
			Type structType)
		{		
			try
			{
				JObject jsonObject = JObject.Parse(jsonString);
				return deserializeFromJson(jsonObject, instance, structType);
			}
			catch (Exception e)
			{
				Console.WriteLine("Failed to deserialize from JSON string: " + e.Message);
				return false;
			}
		}

		public static bool deserializeFromJson<T>(JObject jsonObject, T instance) where T : struct
		{
			return deserializeFromJson(jsonObject, instance, typeof(T));
		}

		public static bool deserializeFromJson(JObject jsonObject, object instance, Type structType)
		{
			try
			{
				var visitor = new JsonReadVisitor(jsonObject);

				Utils.visitObject(instance, structType, visitor);

				return true;
			}
			catch (Exception e)
			{
				Console.WriteLine("Failed to deserialize from JSON: " + e.Message);
				return false;
			}
		}
	}
}
