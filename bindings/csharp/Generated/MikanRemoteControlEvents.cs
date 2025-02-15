// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanAppStageChagedEvent : MikanEvent
	{
		public static new readonly long classId= 6868113282959428280;

		public string new_app_state_name;
		public string old_app_state_name;
	};

	public class MikanRemoteControlEvent : MikanEvent
	{
		public static new readonly long classId= -1767585761143530576;

		public string remoteControlEvent;
		public List<string> parameters;
	};

}
