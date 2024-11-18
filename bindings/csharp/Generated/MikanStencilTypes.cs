// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanStencilModelRenderGeometry
	{
		public List<MikanTriagulatedMesh> meshes { get; set; }
	};

	public struct MikanStencilModelInfo
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }
	};

	public struct MikanTriagulatedMesh
	{
		public List<MikanVector3f> vertices { get; set; }
		public List<MikanVector3f> normals { get; set; }
		public List<MikanVector2f> texels { get; set; }
		public List<int> indices { get; set; }
	};

	public struct MikanStencilQuadInfo
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float quad_width { get; set; }
		public float quad_height { get; set; }
		public bool is_double_sided { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }
	};

	public struct MikanStencilBoxInfo
	{
		public int stencil_id { get; set; }
		public int parent_anchor_id { get; set; }
		public MikanTransform relative_transform { get; set; }
		public float box_x_size { get; set; }
		public float box_y_size { get; set; }
		public float box_z_size { get; set; }
		public bool is_disabled { get; set; }
		public string stencil_name { get; set; }
	};

}
