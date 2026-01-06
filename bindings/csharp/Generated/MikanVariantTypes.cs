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
		FLOAT_TYPE= 3,
		DOUBLE_TYPE= 4,
		MK_STRING_TYPE= 5,
		VECTOR2F_TYPE= 6,
		VECTOR3F_TYPE= 7,
		VECTOR4F_TYPE= 8,
		INT_ARRAY_TYPE= 9,
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

	public class MikanVector2fValue : MikanVariantBase
	{
		public static new readonly long classId= 5487063928969330551;

		public MikanVector2f value;
	};

	public class MikanVector3fValue : MikanVariantBase
	{
		public static new readonly long classId= 3503799446698825680;

		public MikanVector3f value;
	};

	public class MikanVector4fValue : MikanVariantBase
	{
		public static new readonly long classId= -1402910374681859671;

		public MikanVector4f value;
	};

}
