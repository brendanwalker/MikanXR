// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetAppStageInfo : MikanRequest
	{
		public static new readonly long classId= -1337747226464149062;

	};

	public class MikanRemoteControlCommand : MikanRequest
	{
		public static new readonly long classId= 4595909365701644961;

		public string command;
		public List<string> parameters;
	};

	public class MikanRemoteControlCommandResult : MikanResponse
	{
		public static new readonly long classId= -9034381214596710800;

		public List<string> results;
	};

	public class PopAppStage : MikanRequest
	{
		public static new readonly long classId= -589658948136412267;

	};

	public class PushAppStage : MikanRequest
	{
		public static new readonly long classId= -7872424436528764660;

		public string app_state_name;
	};

}
