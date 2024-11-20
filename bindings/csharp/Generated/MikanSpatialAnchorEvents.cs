// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanAnchorNameUpdateEvent : MikanEvent
	{
		public int anchor_id;
		public string anchor_name;
	};

	public class MikanAnchorListUpdateEvent : MikanEvent
	{
	};

	public class MikanAnchorPoseUpdateEvent : MikanEvent
	{
		public MikanTransform transform;
		public int anchor_id;
	};

}
