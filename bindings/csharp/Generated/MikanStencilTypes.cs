// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanStencilModelRenderGeometry
	{
		public List<MikanTriagulatedMesh> meshes;
	};

	public class MikanStencilModelInfo
	{
		public int stencil_id;
		public int parent_anchor_id;
		public MikanTransform relative_transform;
		public bool is_disabled;
		public string stencil_name;
	};

	public class MikanTriagulatedMesh
	{
		public List<MikanVector3f> vertices;
		public List<MikanVector3f> normals;
		public List<MikanVector2f> texels;
		public List<int> indices;
	};

	public class MikanStencilQuadInfo
	{
		public int stencil_id;
		public int parent_anchor_id;
		public MikanTransform relative_transform;
		public float quad_width;
		public float quad_height;
		public bool is_double_sided;
		public bool is_disabled;
		public string stencil_name;
	};

	public class MikanStencilBoxInfo
	{
		public int stencil_id;
		public int parent_anchor_id;
		public MikanTransform relative_transform;
		public float box_x_size;
		public float box_y_size;
		public float box_z_size;
		public bool is_disabled;
		public string stencil_name;
	};

}
