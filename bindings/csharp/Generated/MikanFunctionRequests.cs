// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class FunctionDescriptorResponse : MikanResponse
	{
		public List<MikanFunctionDescriptor> descriptor_list;

		public FunctionDescriptorResponse()
		{
			responseTypeName = "FunctionDescriptorResponse";
		}
	};

	public class GetFunctionListRequest : MikanRequest
	{
		public string systemFilter;
		public string componentFilter;

		public GetFunctionListRequest()
		{
			requestTypeName = "GetFunctionListRequest";
		}
	};

	public class InvokeComponentFunctionRequest : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string functionName;

		public InvokeComponentFunctionRequest()
		{
			requestTypeName = "InvokeComponentFunctionRequest";
		}
	};

	public class InvokeSystemFunctionRequest : MikanRequest
	{
		public string ownerSystem;
		public string functionName;

		public InvokeSystemFunctionRequest()
		{
			requestTypeName = "InvokeSystemFunctionRequest";
		}
	};

}
