using System;
using System.Buffers.Binary;

namespace MikanXR
{
	public class BinaryReader
	{
		private byte[] _readBuffer = null;
		private int _readBufferIndex = 0;

		public BinaryReader(byte[] soruceBuffer)
		{
			_readBuffer= soruceBuffer;
			_readBufferIndex = 0;
		}

		public bool ReadBoolean()
		{
			return ReadByte() > 0;
		}

		public sbyte ReadSByte()
		{
			// Intentionally disable overflow checking for byte -> sbyte
			// See https://simsekahmett.medium.com/understanding-and-using-the-unchecked-keyword-in-c-e6489d6b73aa
			sbyte value = unchecked((sbyte)_readBuffer[_readBufferIndex]);
			_readBufferIndex++;

			return value;
		}

		public byte ReadByte()
		{
			byte value = _readBuffer[_readBufferIndex];
			_readBufferIndex++;

			return value;
		}

		public short ReadInt16()
		{
			short value = BinaryPrimitives.ReadInt16LittleEndian(_readBuffer.AsSpan(_readBufferIndex));
			_readBufferIndex += sizeof(short);

			return value;
		}

		public ushort ReadUInt16()
		{
			ushort value = BinaryPrimitives.ReadUInt16LittleEndian(_readBuffer.AsSpan(_readBufferIndex));
			_readBufferIndex += sizeof(ushort);

			return value;
		}

		public int ReadInt32()
		{
			int value = BinaryPrimitives.ReadInt32LittleEndian(_readBuffer.AsSpan(_readBufferIndex));
			_readBufferIndex += sizeof(int);

			return value;
		}

		public uint ReadUInt32()
		{
			uint value = BinaryPrimitives.ReadUInt32LittleEndian(_readBuffer.AsSpan(_readBufferIndex));
			_readBufferIndex += sizeof(uint);

			return value;
		}

		public long ReadInt64()
		{
			long value = BinaryPrimitives.ReadInt64LittleEndian(_readBuffer.AsSpan(_readBufferIndex));
			_readBufferIndex += sizeof(long);

			return value;
		}

		public ulong ReadUInt64()
		{
			ulong value = BinaryPrimitives.ReadUInt32LittleEndian(_readBuffer.AsSpan(_readBufferIndex));
			_readBufferIndex += sizeof(ulong);

			return value;
		}

		public float ReadSingle()
		{
			float value= BitConverter.ToSingle(_readBuffer, _readBufferIndex);
			_readBufferIndex += sizeof(float);

			return value;
		}

		public double ReadDouble()
		{
			double value= BitConverter.ToDouble(_readBuffer, _readBufferIndex);
			_readBufferIndex += sizeof(float);

			return value;
		}

		public string ReadUTF8String()
		{
			Int32 byteCount = ReadInt32();
			byte[] bytes = ReadBytes(byteCount);

			return System.Text.Encoding.UTF8.GetString(bytes);
		}

		public byte[] ReadBytes(int count)
		{
			return null;
		}
	}
}