namespace MikanXR
{
	public struct MikanEvent
	{
		public string eventType { get; set; }
		
		public MikanEvent()
		{
			eventType= typeof(MikanEvent).Name;
		}

		public MikanEvent(string inEventType)
		{
			eventType= inEventType;
		}
	};
	
	public struct MikanConnectedEvent : MikanEvent
	{
		public MikanConnectedEvent() : MikanEvent(typeof(MikanConnectedEvent).Name) {}
	};

	public struct MikanDisconnectedEvent : MikanEvent
	{
		public MikanDisconnectedEvent() : MikanEvent(typeof(MikanDisconnectedEvent).Name) {}
	};

	public struct MikanVideoSourceOpenedEvent : MikanEvent
	{
		public MikanVideoSourceOpenedEvent() : MikanEvent(typeof(MikanVideoSourceOpenedEvent).Name) {}
	};

	public struct MikanVideoSourceClosedEvent : MikanEvent
	{
		public MikanVideoSourceClosedEvent() : MikanEvent(typeof(MikanVideoSourceClosedEvent).Name) {}
	};

	public struct MikanVideoSourceNewFrameEvent : MikanEvent
	{
		MikanVector3f cameraForward { get; set; }
		MikanVector3f cameraUp { get; set; }
		MikanVector3f cameraPosition { get; set; } 
		uint64_t frame { get; set; }

		public MikanVideoSourceNewFrameEvent() : MikanEvent(typeof(MikanVideoSourceNewFrameEvent).Name) {}
	};

	public struct MikanVideoSourceAttachmentChangedEvent : MikanEvent
	{
		public MikanVideoSourceAttachmentChangedEvent() : MikanEvent(typeof(MikanVideoSourceAttachmentChangedEvent).Name) {}
	};

	public struct MikanVideoSourceIntrinsicsChangedEvent : MikanEvent
	{
		public MikanVideoSourceIntrinsicsChangedEvent() : MikanEvent(typeof(MikanVideoSourceIntrinsicsChangedEvent).Name) {}
	};

	public struct MikanVideoSourceModeChangedEvent : MikanEvent
	{
		public MikanVideoSourceModeChangedEvent() : MikanEvent(typeof(MikanVideoSourceModeChangedEvent).Name) {}
	};

	public struct MikanVRDevicePoseUpdateEvent : MikanEvent
	{
		MikanMatrix4f transform { get; set; }
		MikanVRDeviceID device_id { get; set; }
		uint64_t frame { get; set; }

		public MikanVRDevicePoseUpdateEvent() : MikanEvent(typeof(MikanVRDevicePoseUpdateEvent).Name) {}
	};

	public struct MikanVRDeviceListUpdateEvent : MikanEvent
	{
		public MikanVRDeviceListUpdateEvent() : MikanEvent(typeof(MikanVRDeviceListUpdateEvent).Name) {}
	};

	public struct MikanAnchorPoseUpdateEvent : MikanEvent
	{
		MikanTransform transform { get; set; }
		MikanSpatialAnchorID anchor_id { get; set; }

		public MikanAnchorPoseUpdateEvent() : MikanEvent(typeof(MikanAnchorPoseUpdateEvent).Name) {}
	};

	public struct MikanAnchorListUpdateEvent : MikanEvent
	{
		public MikanAnchorListUpdateEvent() : MikanEvent(typeof(MikanAnchorListUpdateEvent).Name) {}
	};

	public struct MikanScriptMessagePostedEvent : MikanEvent
	{
		std::string message { get; set; }

		public MikanScriptMessagePostedEvent() : MikanEvent(typeof(MikanScriptMessagePostedEvent).Name) {}
	};
	
	
}