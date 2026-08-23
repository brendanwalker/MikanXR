// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetDMXData : MikanRequest
	{
		public List<int> dmx_universe_ids;

		public GetDMXData()
		{
			requestTypeName = "GetDMXData";
		}
	};

	public class MikanDMXDataResponse : MikanResponse
	{
		public MikanDMXData dmx_data;

		public MikanDMXDataResponse()
		{
			responseTypeName = "MikanDMXDataResponse";
		}
	};

	public class SetLightDMXDataSubcription : MikanRequest
	{
		public List<int> light_ids;
		public bool subscribe;

		public SetLightDMXDataSubcription()
		{
			requestTypeName = "SetLightDMXDataSubcription";
		}
	};

}
