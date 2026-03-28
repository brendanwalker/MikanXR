// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class ArucoMarkerImageResponse : MikanResponse
	{
		public string imageData;
	};

	public class GetArucoMarkerImageRequest : MikanRequest
	{
		public int markerId;
		public int imageSize;
	};

}
