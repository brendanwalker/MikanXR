// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanRequest
	{
		public string requestType { get; set; }
		public int requestId { get; set; }
		public int version { get; set; }
	};

	public struct MikanClientInfo
	{
		public string clientId { get; set; }
		public string engineName { get; set; }
		public string engineVersion { get; set; }
		public string applicationName { get; set; }
		public string applicationVersion { get; set; }
		public string xrDeviceName { get; set; }
		public MikanClientGraphicsApi graphicsAPI { get; set; }
		public int mikanCoreSdkVersion { get; set; }
		public bool supportsRGB24 { get; set; }
		public bool supportsRGBA32 { get; set; }
		public bool supportsBGRA32 { get; set; }
		public bool supportsDepth { get; set; }
	};

	public struct MikanResponse
	{
		public string responseType { get; set; }
		public int requestId { get; set; }
		public MikanResult resultCode { get; set; }
	};

	public struct MikanEvent
	{
		public string eventType { get; set; }
	};

}
