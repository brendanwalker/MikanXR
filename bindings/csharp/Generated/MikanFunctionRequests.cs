// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class FunctionDescriptorResponse : MikanResponse
	{
		public List<MikanFunctionDescriptor> descriptor_list;
	};

	public class GetFunctionListRequest : MikanRequest
	{
		public string systemFilter;
		public string componentFilter;
	};

	public class InvokeComponentFunctionRequest : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string functionName;
	};

	public class InvokeSystemFunctionRequest : MikanRequest
	{
		public string ownerSystem;
		public string functionName;
	};

}
