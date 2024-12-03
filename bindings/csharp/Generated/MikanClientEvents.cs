// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanConnectedEvent : MikanEvent
	{
		public static new readonly ulong classId= 9883164577031932740;

		public MikanClientAPIVersion serverVersion;
		public MikanClientAPIVersion minClientVersion;
	};

	public class MikanDisconnectedEvent : MikanEvent
	{
		public static new readonly ulong classId= 13547026039865436498;

		public MikanDisconnectCode code;
		public string reason;
	};

}
