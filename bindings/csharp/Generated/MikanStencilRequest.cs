// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetModelStencilRenderGeometry : MikanRequest
	{
		public int stencilId;

		public GetModelStencilRenderGeometry()
		{
			requestTypeName = "GetModelStencilRenderGeometry";
		}
	};

	public class MikanStencilModelRenderGeometryResponse : MikanResponse
	{
		public MikanStencilModelRenderGeometry render_geometry;

		public MikanStencilModelRenderGeometryResponse()
		{
			responseTypeName = "MikanStencilModelRenderGeometryResponse";
		}
	};

}
