// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanDMXBufferFormat
	{
		DMXUncompressed= 0,
		DMXRLEEncoded= 1,
	};

	public class MikanDMXData
	{
		public double server_time_seconds;
		public List<MikanUniverseDMXData> universes;
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

	public class MikanLightEnvironmentComponentValues : MikanTransformComponentValues
	{
		public List<float> sh_coefficients;
		public float exposure_scale;
		public float directionality;
		public MikanVector3f key_light_direction;
	};

	public class MikanLightEnvironmentSystemValues : MikanSystemValues
	{
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
		public float cone_angle_degrees;
		public float cone_range_meters;
	};

	public class MikanRGBSpotLightSystemValues : MikanSystemValues
	{
	};

	public class MikanUniverseDMXData
	{
		public ushort dmx_universe_id;
		public MikanDMXBufferFormat buffer_format;
		public List<byte> buffer_data;
	};

}
