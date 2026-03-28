// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetAppStageInfo : MikanRequest
	{

		public GetAppStageInfo()
		{
			requestTypeName = "GetAppStageInfo";
		}
	};

	public class MikanAppStageInfoResponse : MikanResponse
	{
		public MikanAppStageInfo app_stage_info;

		public MikanAppStageInfoResponse()
		{
			responseTypeName = "MikanAppStageInfoResponse";
		}
	};

	public class MikanRemoteControlCommand : MikanRequest
	{
		public string command;
		public List<string> parameters;

		public MikanRemoteControlCommand()
		{
			requestTypeName = "MikanRemoteControlCommand";
		}
	};

	public class MikanRemoteControlCommandResult : MikanResponse
	{
		public List<string> results;

		public MikanRemoteControlCommandResult()
		{
			responseTypeName = "MikanRemoteControlCommandResult";
		}
	};

	public class PopAppStage : MikanRequest
	{

		public PopAppStage()
		{
			requestTypeName = "PopAppStage";
		}
	};

	public class PushAppStage : MikanRequest
	{
		public string app_state_name;

		public PushAppStage()
		{
			requestTypeName = "PushAppStage";
		}
	};

}
