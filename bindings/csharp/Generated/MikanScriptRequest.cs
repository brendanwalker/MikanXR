// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class InvokeComponentScriptTrigger : MikanRequest
	{
		public string ownerSystem;
		public int componentId;
		public string trigger_name;
	};

	public class SendScriptMessage : MikanRequest
	{
		public MikanScriptMessageInfo message;
	};

}
