// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanConnectedEvent : MikanEvent
	{
		public MikanClientAPIVersion serverVersion;
		public MikanClientAPIVersion minClientVersion;
		public bool isClientCompatible;

		public MikanConnectedEvent()
		{
			eventTypeName = "MikanConnectedEvent";
		}
	};

	public class MikanDisconnectedEvent : MikanEvent
	{
		public MikanDisconnectCode code;
		public string reason;

		public MikanDisconnectedEvent()
		{
			eventTypeName = "MikanDisconnectedEvent";
		}
	};

}
