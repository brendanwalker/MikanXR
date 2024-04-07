using System.Collections.Generic;

namespace MikanXR
{
	public class MikanStencilQuad : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float quad_width { get; set; }
		public float quad_height { get; set; }
		public bool is_double_sided { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilQuad() : base(typeof(MikanStencilQuad).Name) {}
	};

	public class MikanStencilBox : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float box_x_size { get; set; }
		public float box_y_size { get; set; }
		public float box_z_size { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilBox() : base(typeof(MikanStencilBox).Name) {}
	};

	public class MikanStencilModel : MikanResponse
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; } 
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }

		public MikanStencilModel() : base(typeof(MikanStencilModel).Name) {}
	};

	public class MikanStencilList : MikanResponse
	{
		public List<int> stencil_id_list { get; set; }

		public MikanStencilList() : base(typeof(MikanStencilList).Name) {}
	};
}