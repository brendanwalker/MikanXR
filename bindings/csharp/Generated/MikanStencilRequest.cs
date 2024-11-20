// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetModelStencilList : MikanRequest
	{
	};

	public class GetQuadStencilList : MikanRequest
	{
	};

	public class MikanStencilBoxInfoResponse : MikanResponse
	{
		public MikanStencilBoxInfo box_info;
	};

	public class GetQuadStencil : MikanRequest
	{
		public int stencilId;
	};

	public class MikanStencilQuadInfoResponse : MikanResponse
	{
		public MikanStencilQuadInfo quad_info;
	};

	public class MikanStencilModelRenderGeometryResponse : MikanResponse
	{
		public MikanStencilModelRenderGeometry render_geometry;
	};

	public class MikanStencilModelInfoResponse : MikanResponse
	{
		public MikanStencilModelInfo model_info;
	};

	public class MikanStencilListResponse : MikanResponse
	{
		public List<int> stencil_id_list;
	};

	public class GetBoxStencilList : MikanRequest
	{
	};

	public class GetBoxStencil : MikanRequest
	{
		public int stencilId;
	};

	public class GetModelStencilRenderGeometry : MikanRequest
	{
		public int stencilId;
	};

	public class GetModelStencil : MikanRequest
	{
		public int stencilId;
	};

}
