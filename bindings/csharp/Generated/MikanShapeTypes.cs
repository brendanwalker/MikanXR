// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanBoxShapeComponentValues : MikanShapeComponentValues
	{
		public float box_x_size;
		public float box_y_size;
		public float box_z_size;
	};

	public class MikanBoxShapeSystemValues : MikanSystemValues
	{
	};

	public class MikanModelShapeComponentValues : MikanShapeComponentValues
	{
		public string model_path;
	};

	public class MikanModelShapeSystemValues : MikanSystemValues
	{
	};

	public class MikanQuadShapeComponentValues : MikanShapeComponentValues
	{
		public float quad_width;
		public float quad_height;
		public bool is_double_sided;
	};

	public class MikanQuadShapeSystemValues : MikanSystemValues
	{
	};

	public class MikanShapeComponentValues : MikanTransformComponentValues
	{
		public bool is_disabled;
	};

}
