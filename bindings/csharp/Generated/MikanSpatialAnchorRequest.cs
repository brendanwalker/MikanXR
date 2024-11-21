// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetSpatialAnchorList : MikanRequest
	{
		public static new readonly ulong classId= 7480976529056749966;

	};

	public class GetSpatialAnchorInfo : MikanRequest
	{
		public static new readonly ulong classId= 4372720823303000074;

		public int anchorId;
	};

	public class FindSpatialAnchorInfoByName : MikanRequest
	{
		public static new readonly ulong classId= 10260782275063041579;

		public string anchorName;
	};

}
