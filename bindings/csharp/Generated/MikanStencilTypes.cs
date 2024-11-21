// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanStencilModelRenderGeometry
	{
		public static readonly ulong classId= 6822885306325183796;

		public List<MikanTriagulatedMesh> meshes;
	};

	public class MikanStencilModelInfo
	{
		public static readonly ulong classId= 10393054820209214088;

		public int stencil_id;
		public int parent_anchor_id;
		public MikanTransform relative_transform;
		public bool is_disabled;
		public string stencil_name;
	};

	public class MikanTriagulatedMesh
	{
		public static readonly ulong classId= 16520939264631640594;

		public List<MikanVector3f> vertices;
		public List<MikanVector3f> normals;
		public List<MikanVector2f> texels;
		public List<int> indices;
	};

	public class MikanStencilQuadInfo
	{
		public static readonly ulong classId= 12862572931888908580;

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
		public static readonly ulong classId= 11188967551174986132;

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
