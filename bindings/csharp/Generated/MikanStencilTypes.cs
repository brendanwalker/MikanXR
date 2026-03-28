// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanStencilCullMode
	{
		NONE= 0,
		Z_Axis= 1,
		Y_Axis= 2,
		X_Axis= 3,
	};

	public class MikanBoxStencilComponentValues : MikanStencilComponentValues
	{
		public float box_x_size;
		public float box_y_size;
		public float box_z_size;
	};

	public class MikanBoxStencilSystemValues : MikanSystemValues
	{
		public bool render_stencils;
	};

	public class MikanModelStencilComponentValues : MikanStencilComponentValues
	{
		public string model_path;
	};

	public class MikanModelStencilSystemValues : MikanSystemValues
	{
		public bool render_stencils;
	};

	public class MikanQuadStencilComponentValues : MikanStencilComponentValues
	{
		public float quad_width;
		public float quad_height;
		public bool is_double_sided;
	};

	public class MikanQuadStencilSystemValues : MikanSystemValues
	{
		public bool render_stencils;
	};

	public class MikanStencilComponentValues : MikanTransformComponentValues
	{
		public bool is_disabled;
		public MikanStencilCullMode cull_mode;
	};

	public class MikanStencilModelRenderGeometry
	{
		public List<MikanTriagulatedMesh> meshes;
	};

	public class MikanTriagulatedMesh
	{
		public List<MikanVector3f> vertices;
		public List<MikanVector3f> normals;
		public List<MikanVector2f> texels;
		public List<int> indices;
	};

}
