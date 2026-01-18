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
		public static new readonly long classId= -6788717745397413241;

		public float box_x_size;
		public float box_y_size;
		public float box_z_size;
	};

	public class MikanModelStencilComponentValues : MikanStencilComponentValues
	{
		public static new readonly long classId= 5055554930502929791;

		public string model_path;
	};

	public class MikanQuadStencilComponentValues : MikanStencilComponentValues
	{
		public static new readonly long classId= -9026237790691884165;

		public float quad_width;
		public float quad_height;
		public bool is_double_sided;
	};

	public class MikanStencilComponentValues : MikanTransformComponentValues
	{
		public static new readonly long classId= -4451290801219034056;

		public int parent_anchor_id;
		public bool is_disabled;
		public MikanStencilCullMode cull_mode;
	};

	public class MikanStencilModelRenderGeometry
	{
		public static readonly long classId= 6822885306325183796;

		public List<MikanTriagulatedMesh> meshes;
	};

	public class MikanTriagulatedMesh
	{
		public static readonly long classId= -1925804809077911022;

		public List<MikanVector3f> vertices;
		public List<MikanVector3f> normals;
		public List<MikanVector2f> texels;
		public List<int> indices;
	};

}
