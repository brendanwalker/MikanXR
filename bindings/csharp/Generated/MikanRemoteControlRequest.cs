// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetAppStageInfo : MikanRequest
	{
	};

	public class MikanAppStageInfoResponse : MikanResponse
	{
		public MikanAppStageInfo app_stage_info;
	};

	public class MikanRemoteControlCommand : MikanRequest
	{
		public string command;
		public List<string> parameters;
	};

	public class MikanRemoteControlCommandResult : MikanResponse
	{
		public List<string> results;
	};

	public class PopAppStage : MikanRequest
	{
	};

	public class PushAppStage : MikanRequest
	{
		public string app_state_name;
	};

}
