// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class ArucoMarkerImageResponse : MikanResponse
	{
		public string imageData;

		public ArucoMarkerImageResponse()
		{
			responseTypeName = "ArucoMarkerImageResponse";
		}
	};

	public class GetArucoMarkerImageRequest : MikanRequest
	{
		public int markerId;
		public int imageSize;

		public GetArucoMarkerImageRequest()
		{
			requestTypeName = "GetArucoMarkerImageRequest";
		}
	};

}
