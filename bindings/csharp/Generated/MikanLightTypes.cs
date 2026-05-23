// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanDMXData
	{
		public List<byte> channel_data;
	};

	public class MikanDMXFixtureComponentValues : MikanTransformComponentValues
	{
		public int stage_id;
		public ushort dmx_universe;
		public ushort dmx_start_channel;
		public ushort dmx_channel_count;
		public bool is_disabled;
	};

	public class MikanDMXObjectSystemValues : MikanSystemValues
	{
		public string network_interface_ip;
		public byte dmx_priority;
		public float transmit_rate_hz;
	};

	public class MikanRGBPixelGridComponentValues : MikanDMXFixtureComponentValues
	{
		public int grid_columns;
		public int grid_rows;
	};

	public class MikanRGBPixelGridSystemValues : MikanSystemValues
	{
	};

	public class MikanRGBSpotLightComponentValues : MikanDMXFixtureComponentValues
	{
		public byte red;
		public byte green;
		public byte blue;
	};

	public class MikanRGBSpotLightSystemValues : MikanSystemValues
	{
	};

}
