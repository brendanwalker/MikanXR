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
		INT_ARRAY_TYPE= 17,
		FLOAT_ARRAY_TYPE= 18,
		STRING_ARRAY_TYPE= 19,
		STRING_MAP_TYPE= 20,
		POLYMORPHIC_OBJECT_TYPE= 21,
	};

	public class MikanBoolArrayValue : MikanVariantBase
	{
		public static new readonly long classId= -7544815710777554173;

		public List<bool> value;
	};

	public class MikanBoolValue : MikanVariantBase
	{
		public static new readonly long classId= 2219001884115983454;

		public bool value;
	};

	public class MikanDoubleValue : MikanVariantBase
	{
		public static new readonly long classId= 8832589766978231013;

		public double value;
	};

	public class MikanFloatArrayValue : MikanVariantBase
	{
		public static new readonly long classId= 497562989614948859;

		public List<float> value;
	};

	public class MikanFloatValue : MikanVariantBase
	{
		public static new readonly long classId= 595971514476674198;

		public float value;
	};

	public class MikanIntArrayValue : MikanVariantBase
	{
		public static new readonly long classId= -2537235331547746544;

		public List<int> value;
	};

	public class MikanIntValue : MikanVariantBase
	{
		public static new readonly long classId= 6192287855589871355;

		public int value;
	};

	public class MikanLongValue : MikanVariantBase
	{
		public static new readonly long classId= -2202905653303995628;

		public long value;
	};

	public class MikanMatrix4fValue : MikanVariantBase
	{
		public static new readonly long classId= -2194723481896302537;

		public MikanMatrix4f value;
	};

	public class MikanQuatdValue : MikanVariantBase
	{
		public static new readonly long classId= -303519628627493629;

		public MikanQuatd value;
	};

	public class MikanQuatfValue : MikanVariantBase
	{
		public static new readonly long classId= -3980278345028742771;

		public MikanQuatf value;
	};

	public class MikanStringArrayValue : MikanVariantBase
	{
		public static new readonly long classId= 5781178291974097454;

		public List<string> value;
	};

	public class MikanStringMapValue : MikanVariantBase
	{
		public static new readonly long classId= -5220301160931703877;

		public Dictionary<string, string> value;
	};

	public class MikanStringValue : MikanVariantBase
	{
		public static new readonly long classId= -318636760475246811;

		public string value;
	};

	public class MikanVariant
	{
		public static readonly long classId= -8543347830987565886;

		public MikanVariantType value_type;
		public PolymorphicObject value_ptr;
	};

	public class MikanVariantBase : PolymorphicStruct
	{
		public static new readonly long classId= 5706978007370628991;

	};

	public class MikanVector2dValue : MikanVariantBase
	{
		public static new readonly long classId= -3597335519782313151;

		public MikanVector2d value;
	};

	public class MikanVector2fValue : MikanVariantBase
	{
		public static new readonly long classId= 5487063928969330551;

		public MikanVector2f value;
	};

	public class MikanVector3dValue : MikanVariantBase
	{
		public static new readonly long classId= 905231883766189926;

		public MikanVector3d value;
	};

	public class MikanVector3fValue : MikanVariantBase
	{
		public static new readonly long classId= 3503799446698825680;

		public MikanVector3f value;
	};

	public class MikanVector4dValue : MikanVariantBase
	{
		public static new readonly long classId= 398000504403518783;

		public MikanVector4d value;
	};

	public class MikanVector4fValue : MikanVariantBase
	{
		public static new readonly long classId= -1402910374681859671;

		public MikanVector4f value;
	};

}
