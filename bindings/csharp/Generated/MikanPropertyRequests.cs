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
		public string ownerSystem;
		public int componentId;
	};

	public class ComponentGetValuesResponse : MikanResponse
	{
		public string ownerSystem;
		public string componentClassName;
		public PolymorphicObject valuesObject;
	};

	public class ComponentListResponse : MikanResponse
	{
		public List<int> componentIdList;
	};

	public class GetComponentListRequest : MikanRequest
	{
		public string ownerSystem;
		public string componentClassName;
	};

	public class GetPropertyDescriptors : MikanRequest
	{
		public string systemFilter;
		public string componentFilter;
		public string propertyFilter;
	};

	public class PropertyDescriptorResponse : MikanResponse
	{
		public List<MikanPropertyDescriptor> descriptor_list;
	};

	public class PropertyGetValueRequest : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string fieldName;
	};

	public class PropertyGetValueResponse : MikanResponse
	{
		public MikanPropertyValue propertyValue;
	};

	public class PropertySetValueRequest : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string fieldName;
		public MikanVariant fieldValue;
	};

	public class PropertySetValueResponse : MikanResponse
	{
	};

	public class SetPropertyNotifyMode : MikanRequest
	{
		public string systemFilter;
		public string componentFilter;
		public string propertyFilter;
		public MikanPropertyNotifyMode notifyMode;
	};

	public class SystemCreateObjectRequest : MikanRequest
	{
		public string ownerSystem;
		public string componentClassName;
		public PolymorphicObject initParams;
	};

	public class SystemDestroyObjectRequest : MikanRequest
	{
		public string ownerSystem;
		public string componentClassName;
		public int componentId;
	};

	public class SystemGetValuesRequest : MikanRequest
	{
		public string ownerSystem;
	};

	public class SystemGetValuesResponse : MikanResponse
	{
		public string ownerSystem;
		public PolymorphicObject valuesObject;
	};

}
