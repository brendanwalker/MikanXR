using System;

namespace MikanXR
{
	public class MikanEvent
	{
		public string eventType { get; set; } = "";
		
		public MikanEvent()
		{
			eventType= typeof(MikanEvent).Name;
		}

		public MikanEvent(string inEventType)
		{
			eventType= inEventType;
		}
	};
	
	public class MikanConnectedEvent : MikanEvent
	{
		public MikanConnectedEvent() : base(typeof(MikanConnectedEvent).Name) {}
	};

	public class MikanDisconnectedEvent : MikanEvent
	{
		public MikanDisconnectedEvent() : base(typeof(MikanDisconnectedEvent).Name) {}
	};

	public class MikanVideoSourceOpenedEvent : MikanEvent
	{
		public MikanVideoSourceOpenedEvent() : base(typeof(MikanVideoSourceOpenedEvent).Name) {}
	};

	public class MikanVideoSourceClosedEvent : MikanEvent
	{
		public MikanVideoSourceClosedEvent() : base(typeof(MikanVideoSourceClosedEvent).Name) {}
	};

	public class MikanVideoSourceNewFrameEvent : MikanEvent
	{
		public MikanVector3f cameraForward { get; set; }
		public MikanVector3f cameraUp { get; set; }
		public MikanVector3f cameraPosition { get; set; } 
		public UInt64 frame { get; set; }

		public MikanVideoSourceNewFrameEvent() : base(typeof(MikanVideoSourceNewFrameEvent).Name) {}
	};

	public class MikanVideoSourceAttachmentChangedEvent : MikanEvent
	{
		public MikanVideoSourceAttachmentChangedEvent() : base(typeof(MikanVideoSourceAttachmentChangedEvent).Name) {}
	};

	public class MikanVideoSourceIntrinsicsChangedEvent : MikanEvent
	{
		public MikanVideoSourceIntrinsicsChangedEvent() : base(typeof(MikanVideoSourceIntrinsicsChangedEvent).Name) {}
	};

	public class MikanVideoSourceModeChangedEvent : MikanEvent
	{
		public MikanVideoSourceModeChangedEvent() : base(typeof(MikanVideoSourceModeChangedEvent).Name) {}
	};

	public class MikanVRDevicePoseUpdateEvent : MikanEvent
	{
		public MikanMatrix4f transform { get; set; }
		public int device_id { get; set; }
		public UInt64 frame { get; set; }

		public MikanVRDevicePoseUpdateEvent() : base(typeof(MikanVRDevicePoseUpdateEvent).Name) {}
	};

	public class MikanVRDeviceListUpdateEvent : MikanEvent
	{
		public MikanVRDeviceListUpdateEvent() : base(typeof(MikanVRDeviceListUpdateEvent).Name) {}
	};

	public class MikanAnchorPoseUpdateEvent : MikanEvent
	{
		public MikanTransform transform { get; set; }
		public int anchor_id { get; set; }

		public MikanAnchorPoseUpdateEvent() : base(typeof(MikanAnchorPoseUpdateEvent).Name) {}
	};

	public class MikanAnchorListUpdateEvent : MikanEvent
	{
		public MikanAnchorListUpdateEvent() : base(typeof(MikanAnchorListUpdateEvent).Name) {}
	};

	public class MikanScriptMessagePostedEvent : MikanEvent
	{
		public string message { get; set; }

		public MikanScriptMessagePostedEvent() : base(typeof(MikanScriptMessagePostedEvent).Name) {}
	};
	
	
}