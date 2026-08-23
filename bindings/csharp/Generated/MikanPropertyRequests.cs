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

		public ComponentGetValuesRequest()
		{
			requestTypeName = "ComponentGetValuesRequest";
		}
	};

	public class ComponentGetValuesResponse : MikanResponse
	{
		public string ownerSystem;
		public string componentClassName;
		public PolymorphicObject valuesObject;

		public ComponentGetValuesResponse()
		{
			responseTypeName = "ComponentGetValuesResponse";
		}
	};

	public class ComponentListResponse : MikanResponse
	{
		public List<int> componentIdList;

		public ComponentListResponse()
		{
			responseTypeName = "ComponentListResponse";
		}
	};

	public class GetComponentListRequest : MikanRequest
	{
		public string ownerSystem;
		public string componentClassName;

		public GetComponentListRequest()
		{
			requestTypeName = "GetComponentListRequest";
		}
	};

	public class GetPropertyDescriptors : MikanRequest
	{
		public string systemFilter;
		public string componentFilter;
		public string propertyFilter;

		public GetPropertyDescriptors()
		{
			requestTypeName = "GetPropertyDescriptors";
		}
	};

	public class PropertyDescriptorResponse : MikanResponse
	{
		public List<MikanPropertyDescriptor> descriptor_list;

		public PropertyDescriptorResponse()
		{
			responseTypeName = "PropertyDescriptorResponse";
		}
	};

	public class PropertyGetValueRequest : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string fieldName;

		public PropertyGetValueRequest()
		{
			requestTypeName = "PropertyGetValueRequest";
		}
	};

	public class PropertyGetValueResponse : MikanResponse
	{
		public MikanPropertyValue propertyValue;

		public PropertyGetValueResponse()
		{
			responseTypeName = "PropertyGetValueResponse";
		}
	};

	public class PropertySetValueRequest : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string fieldName;
		public MikanVariant fieldValue;

		public PropertySetValueRequest()
		{
			requestTypeName = "PropertySetValueRequest";
		}
	};

	public class PropertySetValueResponse : MikanResponse
	{

		public PropertySetValueResponse()
		{
			responseTypeName = "PropertySetValueResponse";
		}
	};

	public class SetPropertyNotifyMode : MikanRequest
	{
		public string systemFilter;
		public string componentFilter;
		public string propertyFilter;
		public MikanPropertyNotifyMode notifyMode;

		public SetPropertyNotifyMode()
		{
			requestTypeName = "SetPropertyNotifyMode";
		}
	};

	public class SystemCreateObjectRequest : MikanRequest
	{
		public string ownerSystem;
		public string componentClassName;
		public PolymorphicObject initParams;

		public SystemCreateObjectRequest()
		{
			requestTypeName = "SystemCreateObjectRequest";
		}
	};

	public class SystemDestroyObjectRequest : MikanRequest
	{
		public string ownerSystem;
		public string componentClassName;
		public int componentId;

		public SystemDestroyObjectRequest()
		{
			requestTypeName = "SystemDestroyObjectRequest";
		}
	};

	public class SystemGetValuesRequest : MikanRequest
	{
		public string ownerSystem;

		public SystemGetValuesRequest()
		{
			requestTypeName = "SystemGetValuesRequest";
		}
	};

	public class SystemGetValuesResponse : MikanResponse
	{
		public string ownerSystem;
		public PolymorphicObject valuesObject;

		public SystemGetValuesResponse()
		{
			responseTypeName = "SystemGetValuesResponse";
		}
	};

}
