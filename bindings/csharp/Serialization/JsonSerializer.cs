using Newtonsoft.Json.Linq;
using System;

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

			}

			public void visitEnum(ValueAccessor accessor)
			{

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
