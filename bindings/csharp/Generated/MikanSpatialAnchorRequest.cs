// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct GetSpatialAnchorList : MikanRequest
	{
	};

	public struct GetSpatialAnchorInfo : MikanRequest
	{
		public int anchorId { get; set; }
	};

	public struct FindSpatialAnchorInfoByName : MikanRequest
	{
		public string anchorName { get; set; }
	};

}
