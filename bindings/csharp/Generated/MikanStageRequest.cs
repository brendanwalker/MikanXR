// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetStage : MikanRequest
	{
		public static new readonly long classId= -7321858344034235307;

		public int stage_id;
	};

	public class GetStageList : MikanRequest
	{
		public static new readonly long classId= -5808966371246121775;

	};

	public class MikanStageInfoResponse : MikanResponse
	{
		public static new readonly long classId= -6952891508631852806;

		public MikanStageInfo stage_info;
	};

	public class MikanStageListResponse : MikanResponse
	{
		public static new readonly long classId= 3664860834360875758;

		public List<int> stage_id_list;
	};

}
