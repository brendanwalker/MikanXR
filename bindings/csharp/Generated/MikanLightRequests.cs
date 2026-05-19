// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class CreateRGBPixelGrid : MikanRequest
	{

		public CreateRGBPixelGrid()
		{
			requestTypeName = "CreateRGBPixelGrid";
		}
	};

	public class CreateRGBSpotLight : MikanRequest
	{

		public CreateRGBSpotLight()
		{
			requestTypeName = "CreateRGBSpotLight";
		}
	};

	public class DestroyRGBPixelGrid : MikanRequest
	{
		public int grid_id;

		public DestroyRGBPixelGrid()
		{
			requestTypeName = "DestroyRGBPixelGrid";
		}
	};

	public class DestroyRGBSpotLight : MikanRequest
	{
		public int light_id;

		public DestroyRGBSpotLight()
		{
			requestTypeName = "DestroyRGBSpotLight";
		}
	};

	public class GetRGBPixelGrid : MikanRequest
	{
		public int grid_id;

		public GetRGBPixelGrid()
		{
			requestTypeName = "GetRGBPixelGrid";
		}
	};

	public class GetRGBPixelGridList : MikanRequest
	{

		public GetRGBPixelGridList()
		{
			requestTypeName = "GetRGBPixelGridList";
		}
	};

	public class GetRGBSpotLight : MikanRequest
	{
		public int light_id;

		public GetRGBSpotLight()
		{
			requestTypeName = "GetRGBSpotLight";
		}
	};

	public class GetRGBSpotLightList : MikanRequest
	{

		public GetRGBSpotLightList()
		{
			requestTypeName = "GetRGBSpotLightList";
		}
	};

	public class MikanRGBPixelGridListResponse : MikanResponse
	{
		public List<int> grid_ids;

		public MikanRGBPixelGridListResponse()
		{
			responseTypeName = "MikanRGBPixelGridListResponse";
		}
	};

	public class MikanRGBPixelGridResponse : MikanResponse
	{
		public MikanRGBPixelGridComponentValues grid_info;

		public MikanRGBPixelGridResponse()
		{
			responseTypeName = "MikanRGBPixelGridResponse";
		}
	};

	public class MikanRGBSpotLightListResponse : MikanResponse
	{
		public List<int> light_ids;

		public MikanRGBSpotLightListResponse()
		{
			responseTypeName = "MikanRGBSpotLightListResponse";
		}
	};

	public class MikanRGBSpotLightResponse : MikanResponse
	{
		public MikanRGBSpotLightComponentValues light_info;

		public MikanRGBSpotLightResponse()
		{
			responseTypeName = "MikanRGBSpotLightResponse";
		}
	};

	public class SetRGBPixelGridData : MikanRequest
	{
		public int grid_id;
		public List<byte> pixel_data;

		public SetRGBPixelGridData()
		{
			requestTypeName = "SetRGBPixelGridData";
		}
	};

}
