// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetLightDMXData : MikanRequest
	{
		public int light_id;

		public GetLightDMXData()
		{
			requestTypeName = "GetLightDMXData";
		}
	};

	public class MikanLightDMXDataResponse : MikanResponse
	{
		public int light_id;
		public MikanDMXData dmx_data;

		public MikanLightDMXDataResponse()
		{
			responseTypeName = "MikanLightDMXDataResponse";
		}
	};

	public class SetLightDMXData : MikanRequest
	{
		public int light_id;
		public MikanDMXData dmx_data;

		public SetLightDMXData()
		{
			requestTypeName = "SetLightDMXData";
		}
	};

	public class SetLightDMXDataSubcription : MikanRequest
	{
		public int light_id;
		public bool subscribe;

		public SetLightDMXDataSubcription()
		{
			requestTypeName = "SetLightDMXDataSubcription";
		}
	};

}
