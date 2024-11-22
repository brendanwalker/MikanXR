using System;
using System.IO;

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

			}

			public void visitList(ValueAccessor accessor)
			{

			}

			public void visitDictionary(ValueAccessor accessor)
			{

			}

			public void visitPolymorphicObject(ValueAccessor accessor)
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

			public void visitString(ValueAccessor accessor)
			{

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
