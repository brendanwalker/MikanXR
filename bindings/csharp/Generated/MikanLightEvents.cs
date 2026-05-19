// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanRGBPixelGridCreatedEvent : MikanEvent
	{
		public int grid_id;

		public MikanRGBPixelGridCreatedEvent()
		{
			eventTypeName = "MikanRGBPixelGridCreatedEvent";
		}
	};

	public class MikanRGBPixelGridDeletedEvent : MikanEvent
	{
		public int grid_id;

		public MikanRGBPixelGridDeletedEvent()
		{
			eventTypeName = "MikanRGBPixelGridDeletedEvent";
		}
	};

	public class MikanRGBSpotLightCreatedEvent : MikanEvent
	{
		public int light_id;

		public MikanRGBSpotLightCreatedEvent()
		{
			eventTypeName = "MikanRGBSpotLightCreatedEvent";
		}
	};

	public class MikanRGBSpotLightDeletedEvent : MikanEvent
	{
		public int light_id;

		public MikanRGBSpotLightDeletedEvent()
		{
			eventTypeName = "MikanRGBSpotLightDeletedEvent";
		}
	};

}
