using System.Collections.Generic;

namespace MikanXR
{
	public class MikanStencilQuadInfo : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float quad_width { get; set; }
		public float quad_height { get; set; }
		public bool is_double_sided { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilQuadInfo() : base(typeof(MikanStencilQuadInfo).Name) {}
	};

	public class MikanStencilBoxInfo : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float box_x_size { get; set; }
		public float box_y_size { get; set; }
		public float box_z_size { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilBoxInfo() : base(typeof(MikanStencilBoxInfo).Name) {}
	};

	public class MikanStencilModelInfo : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; } 
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilModelInfo() : base(typeof(MikanStencilModelInfo).Name) {}
	};

	public class MikanStencilList : MikanResponse
	{
		public List<int> stencil_id_list { get; set; }

		public MikanStencilList() : base(typeof(MikanStencilList).Name) {}
	};
}