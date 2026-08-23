// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class DisposeClientRequest : MikanRequest
	{
		public string clientId;

		public DisposeClientRequest()
		{
			requestTypeName = "DisposeClientRequest";
		}
	};

	public class InitClientRequest : MikanRequest
	{
		public MikanClientInfo clientInfo;

		public InitClientRequest()
		{
			requestTypeName = "InitClientRequest";
		}
	};

}
