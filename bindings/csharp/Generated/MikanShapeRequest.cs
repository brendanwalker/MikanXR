// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetModelShapeRenderGeometry : MikanRequest
	{
		public int shapeId;

		public GetModelShapeRenderGeometry()
		{
			requestTypeName = "GetModelShapeRenderGeometry";
		}
	};

	public class MikanShapeModelRenderGeometryResponse : MikanResponse
	{
		public MikanStencilModelRenderGeometry render_geometry;

		public MikanShapeModelRenderGeometryResponse()
		{
			responseTypeName = "MikanShapeModelRenderGeometryResponse";
		}
	};

}
