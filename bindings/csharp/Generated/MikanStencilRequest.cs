// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class GetModelStencilList : MikanRequest
	{
		public static new readonly ulong classId= 11450594457159259050;

	};

	public class GetQuadStencilList : MikanRequest
	{
		public static new readonly ulong classId= 1562088813869505254;

	};

	public class MikanStencilBoxInfoResponse : MikanResponse
	{
		public static new readonly ulong classId= 8416540735772150643;

		public MikanStencilBoxInfo box_info;
	};

	public class GetQuadStencil : MikanRequest
	{
		public static new readonly ulong classId= 1551069120606568234;

		public int stencilId;
	};

	public class MikanStencilQuadInfoResponse : MikanResponse
	{
		public static new readonly ulong classId= 5745901639841426435;

		public MikanStencilQuadInfo quad_info;
	};

	public class MikanStencilModelRenderGeometryResponse : MikanResponse
	{
		public static new readonly ulong classId= 6128619420232158675;

		public MikanStencilModelRenderGeometry render_geometry;
	};

	public class MikanStencilModelInfoResponse : MikanResponse
	{
		public static new readonly ulong classId= 10493546222299065415;

		public MikanStencilModelInfo model_info;
	};

	public class MikanStencilListResponse : MikanResponse
	{
		public static new readonly ulong classId= 17295012601112148364;

		public List<int> stencil_id_list;
	};

	public class GetBoxStencilList : MikanRequest
	{
		public static new readonly ulong classId= 497666200403468338;

	};

	public class GetBoxStencil : MikanRequest
	{
		public static new readonly ulong classId= 10469116181362662566;

		public int stencilId;
	};

	public class GetModelStencilRenderGeometry : MikanRequest
	{
		public static new readonly ulong classId= 7106057332746101286;

		public int stencilId;
	};

	public class GetModelStencil : MikanRequest
	{
		public static new readonly ulong classId= 2139398247341621214;

		public int stencilId;
	};

}
