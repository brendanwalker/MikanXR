// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanQuadStencilListUpdateEvent : MikanEvent
	{
	};

	public class MikanModelStencilListUpdateEvent : MikanEvent
	{
	};

	public class MikanStencilNameUpdateEvent : MikanEvent
	{
		public int stencil_id;
		public string stencil_name;
	};

	public class MikanStencilPoseUpdateEvent : MikanEvent
	{
		public MikanTransform transform;
		public int stencil_id;
	};

	public class MikanBoxStencilListUpdateEvent : MikanEvent
	{
	};

}
