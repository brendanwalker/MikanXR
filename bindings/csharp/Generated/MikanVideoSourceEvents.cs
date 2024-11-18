// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanVideoSourceOpenedEvent : MikanEvent
	{
	};

	public struct MikanVideoSourceClosedEvent : MikanEvent
	{
	};

	public struct MikanVideoSourceNewFrameEvent : MikanEvent
	{
		public MikanVector3f cameraForward { get; set; }
		public MikanVector3f cameraUp { get; set; }
		public MikanVector3f cameraPosition { get; set; }
		public ulong frame { get; set; }
	};

	public struct MikanVideoSourceAttachmentChangedEvent : MikanEvent
	{
	};

	public struct MikanVideoSourceIntrinsicsChangedEvent : MikanEvent
	{
	};

	public struct MikanVideoSourceModeChangedEvent : MikanEvent
	{
	};

}
