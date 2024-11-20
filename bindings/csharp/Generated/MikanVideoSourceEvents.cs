// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanVideoSourceOpenedEvent : MikanEvent
	{
	};

	public class MikanVideoSourceClosedEvent : MikanEvent
	{
	};

	public class MikanVideoSourceNewFrameEvent : MikanEvent
	{
		public MikanVector3f cameraForward;
		public MikanVector3f cameraUp;
		public MikanVector3f cameraPosition;
		public ulong frame;
	};

	public class MikanVideoSourceAttachmentChangedEvent : MikanEvent
	{
	};

	public class MikanVideoSourceIntrinsicsChangedEvent : MikanEvent
	{
	};

	public class MikanVideoSourceModeChangedEvent : MikanEvent
	{
	};

}
