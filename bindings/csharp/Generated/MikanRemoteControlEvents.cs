// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanAppStageChangedEvent : MikanEvent
	{
		public string new_app_state_name;
		public string old_app_state_name;
	};

	public class MikanRemoteControlEvent : MikanEvent
	{
		public string remoteControlEvent;
		public List<string> parameters;
	};

}
