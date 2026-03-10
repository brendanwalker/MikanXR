// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class FunctionDescriptorResponse : MikanResponse
	{
		public static new readonly long classId= -2965742291485466747;

		public List<MikanFunctionDescriptor> descriptor_list;
	};

	public class GetFunctionListRequest : MikanRequest
	{
		public static new readonly long classId= 5674758152299327690;

		public string systemFilter;
		public string componentFilter;
	};

	public class InvokeComponentFunctionRequest : MikanRequest
	{
		public static new readonly long classId= -7601310242316033921;

		public string ownerSystem;
		public int componentId;
		public string functionName;
	};

	public class InvokeSystemFunctionRequest : MikanRequest
	{
		public static new readonly long classId= 5600332656811306803;

		public string ownerSystem;
		public string functionName;
	};

}
