// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class ArucoMarkerImageResponse : MikanResponse
	{
		public static new readonly long classId= -9130171779594453109;

		public string imageData;
	};

	public class GetArucoMarkerImageRequest : MikanRequest
	{
		public static new readonly long classId= -8657100980457496105;

		public int markerId;
		public int imageSize;
	};

}
