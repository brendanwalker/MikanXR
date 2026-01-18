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

	public class ComponentGetValuesRequest : MikanRequest
	{
		public static new readonly long classId= -4733146441529199375;

		public string ownerSystem;
		public int componentId;
	};

	public class ComponentGetValuesResponse : MikanResponse
	{
		public static new readonly long classId= -2056830896783471569;

		public string ownerSystem;
		public string componentClassName;
		public PolymorphicObject valuesObject;
	};

	public class ComponentListResponse : MikanResponse
	{
		public static new readonly long classId= 6592714266827556333;

		public List<int> componentIdList;
	};

	public class GetComponentListRequest : MikanRequest
	{
		public static new readonly long classId= 6023810896618378113;

		public string ownerSystem;
		public string componentClassName;
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

	public class SystemGetValuesRequest : MikanRequest
	{
		public static new readonly long classId= -8302559874499546923;

		public string ownerSystem;
	};

	public class SystemGetValuesResponse : MikanResponse
	{
		public static new readonly long classId= -688309113082437205;

		public string ownerSystem;
		public PolymorphicObject valuesObject;
	};

}
