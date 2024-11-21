using System;
using System.IO;

namespace MikanXR
{
	public class BinarySerializer
	{
		private class BinaryWriteVisitor : IVisitor
		{
			private BinaryWriter _writer;

			public BinaryWriteVisitor(BinaryWriter writer)
			{
				_writer = writer;
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

		public static byte[] SerializeToBytes<T>(T instance) where T : class
		{
			return SerializeToBytes(instance, typeof(T));
		}

		public static byte[] SerializeToBytes(object instance, Type structType)
		{
			var memoryStream = new MemoryStream();
			var writer = new BinaryWriter(memoryStream);
			var visitor = new BinaryWriteVisitor(writer);

			Utils.visitObject(instance, structType, visitor);

			return memoryStream.ToArray();
		}
	}
}
