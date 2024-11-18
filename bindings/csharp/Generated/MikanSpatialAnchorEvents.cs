// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanAnchorNameUpdateEvent : MikanEvent
	{
		public int anchor_id { get; set; }
		public string anchor_name { get; set; }
	};

	public struct MikanAnchorListUpdateEvent : MikanEvent
	{
	};

	public struct MikanAnchorPoseUpdateEvent : MikanEvent
	{
		public MikanTransform transform { get; set; }
		public int anchor_id { get; set; }
	};

}
