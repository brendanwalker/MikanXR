// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanPropertyNotifyMode
	{
		NONE= 0,
		NAME= 1,
		NAME_AND_VALUE= 2,
	};

	public class GetPropertyDescriptors : MikanRequest
	{
		public static new readonly long classId= 5150175307679594166;

		public string systemFilter;
		public string componentFilter;
		public string propertyFilter;
	};

	public class PropertyDescriptorResponse : MikanResponse
	{
		public static new readonly long classId= 6963368381922795052;

		public List<MikanPropertyDescriptor> descriptor_list;
	};

	public class PropertyGetValueRequest : MikanRequest
	{
		public static new readonly long classId= 901735839132130952;

		public string ownerSystem;
		public int componentId;
		public string fieldName;
	};

	public class PropertyGetValueResponse : MikanResponse
	{
		public static new readonly long classId= 8945416611226246596;

		public MikanPropertyValue propertyValue;
	};

	public class PropertySetValueRequest : MikanRequest
	{
		public static new readonly long classId= -1776666409656556548;

		public string ownerSystem;
		public int componentId;
		public string fieldName;
		public MikanVariant fieldValue;
	};

	public class PropertySetValueResponse : MikanResponse
	{
		public static new readonly long classId= 1995658389030653232;

	};

	public class SetPropertyNotifyMode : MikanRequest
	{
		public static new readonly long classId= -7219101800050484750;

		public string systemFilter;
		public string componentFilter;
		public string propertyFilter;
		public MikanPropertyNotifyMode notifyMode;
	};

}
