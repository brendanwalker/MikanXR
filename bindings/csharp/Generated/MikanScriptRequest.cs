// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class InvokeComponentScriptTrigger : MikanRequest
	{
		public static new readonly long classId= -8350650813685957365;

		public string ownerSystem;
		public int componentId;
		public string trigger_name;
	};

	public class SendScriptMessage : MikanRequest
	{
		public static new readonly long classId= -3006836539234531471;

		public MikanScriptMessageInfo message;
	};

}
