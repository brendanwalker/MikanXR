// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanVariantType
	{
		INVALID_TYPE= 0,
		BOOL_TYPE= 1,
		INT_TYPE= 2,
		LONG_TYPE= 3,
		FLOAT_TYPE= 4,
		DOUBLE_TYPE= 5,
		MK_STRING_TYPE= 6,
		VECTOR2F_TYPE= 7,
		VECTOR3F_TYPE= 8,
		VECTOR4F_TYPE= 9,
		QUATERNIONF_TYPE= 10,
		MATRIX4F_TYPE= 11,
		VECTOR2D_TYPE= 12,
		VECTOR3D_TYPE= 13,
		VECTOR4D_TYPE= 14,
		QUATERNIOND_TYPE= 15,
		BOOL_ARRAY_TYPE= 16,
		UBYTE_ARRAY_TYPE= 17,
		INT_ARRAY_TYPE= 18,
		FLOAT_ARRAY_TYPE= 19,
		STRING_ARRAY_TYPE= 20,
		STRING_MAP_TYPE= 21,
		POLYMORPHIC_OBJECT_TYPE= 22,
	};

	public class MikanBoolArrayValue : MikanVariantBase
	{
		public List<bool> value;
	};

	public class MikanBoolValue : MikanVariantBase
	{
		public bool value;
	};

	public class MikanDoubleValue : MikanVariantBase
	{
		public double value;
	};

	public class MikanFloatArrayValue : MikanVariantBase
	{
		public List<float> value;
	};

	public class MikanFloatValue : MikanVariantBase
	{
		public float value;
	};

	public class MikanIntArrayValue : MikanVariantBase
	{
		public List<int> value;
	};

	public class MikanIntValue : MikanVariantBase
	{
		public int value;
	};

	public class MikanLongValue : MikanVariantBase
	{
		public long value;
	};

	public class MikanMatrix4fValue : MikanVariantBase
	{
		public MikanMatrix4f value;
	};

	public class MikanQuatdValue : MikanVariantBase
	{
		public MikanQuatd value;
	};

	public class MikanQuatfValue : MikanVariantBase
	{
		public MikanQuatf value;
	};

	public class MikanStringArrayValue : MikanVariantBase
	{
		public List<string> value;
	};

	public class MikanStringMapValue : MikanVariantBase
	{
		public Dictionary<string, string> value;
	};

	public class MikanStringValue : MikanVariantBase
	{
		public string value;
	};

	public class MikanUByteArrayValue : MikanVariantBase
	{
		public List<byte> value;
	};

	public class MikanVariant
	{
		public MikanVariantType value_type;
		public PolymorphicObject value_ptr;
	};

	public class MikanVariantBase : PolymorphicStruct
	{
	};

	public class MikanVector2dValue : MikanVariantBase
	{
		public MikanVector2d value;
	};

	public class MikanVector2fValue : MikanVariantBase
	{
		public MikanVector2f value;
	};

	public class MikanVector3dValue : MikanVariantBase
	{
		public MikanVector3d value;
	};

	public class MikanVector3fValue : MikanVariantBase
	{
		public MikanVector3f value;
	};

	public class MikanVector4dValue : MikanVariantBase
	{
		public MikanVector4d value;
	};

	public class MikanVector4fValue : MikanVariantBase
	{
		public MikanVector4f value;
	};

}
