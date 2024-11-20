// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanRequest
	{
		public string requestType;
		public int requestId;
		public int version;
	};

	public class MikanClientInfo
	{
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
		public string responseType;
		public int requestId;
		public MikanResult resultCode;
	};

	public class MikanEvent
	{
		public string eventType;
	};

}
