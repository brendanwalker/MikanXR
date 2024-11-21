// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanRequest
	{
		public static readonly ulong classId= 1095719431187359814;

		public string requestType;
		public int requestId;
		public int version;
	};

	public class MikanClientInfo
	{
		public static readonly ulong classId= 16144647093330338146;

		public string clientId;
		public string engineName;
		public string engineVersion;
		public string applicationName;
		public string applicationVersion;
		public string xrDeviceName;
		public MikanClientGraphicsApi graphicsAPI;
		public int mikanCoreSdkVersion;
		public bool supportsRGB24;
		public bool supportsRGBA32;
		public bool supportsBGRA32;
		public bool supportsDepth;
	};

	public class MikanResponse
	{
		public static readonly ulong classId= 7094118849615581562;

		public string responseType;
		public int requestId;
		public MikanResult resultCode;
	};

	public class MikanEvent
	{
		public static readonly ulong classId= 8521159033538382795;

		public string eventType;
	};

}
