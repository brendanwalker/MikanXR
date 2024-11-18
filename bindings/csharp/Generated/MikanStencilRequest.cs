// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct GetModelStencilList : MikanRequest
	{
	};

	public struct GetQuadStencilList : MikanRequest
	{
	};

	public struct MikanStencilBoxInfoResponse : MikanResponse
	{
		public MikanStencilBoxInfo box_info { get; set; }
	};

	public struct GetQuadStencil : MikanRequest
	{
		public int stencilId { get; set; }
	};

	public struct MikanStencilQuadInfoResponse : MikanResponse
	{
		public MikanStencilQuadInfo quad_info { get; set; }
	};

	public struct MikanStencilModelRenderGeometryResponse : MikanResponse
	{
		public MikanStencilModelRenderGeometry render_geometry { get; set; }
	};

	public struct MikanStencilModelInfoResponse : MikanResponse
	{
		public MikanStencilModelInfo model_info { get; set; }
	};

	public struct MikanStencilListResponse : MikanResponse
	{
		public List<int> stencil_id_list { get; set; }
	};

	public struct GetBoxStencilList : MikanRequest
	{
	};

	public struct GetBoxStencil : MikanRequest
	{
		public int stencilId { get; set; }
	};

	public struct GetModelStencilRenderGeometry : MikanRequest
	{
		public int stencilId { get; set; }
	};

	public struct GetModelStencil : MikanRequest
	{
		public int stencilId { get; set; }
	};

}
