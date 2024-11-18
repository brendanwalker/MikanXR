// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct MikanQuadStencilListUpdateEvent : MikanEvent
	{
	};

	public struct MikanModelStencilListUpdateEvent : MikanEvent
	{
	};

	public struct MikanStencilNameUpdateEvent : MikanEvent
	{
		public int stencil_id { get; set; }
		public string stencil_name { get; set; }
	};

	public struct MikanStencilPoseUpdateEvent : MikanEvent
	{
		public MikanTransform transform { get; set; }
		public int stencil_id { get; set; }
	};

	public struct MikanBoxStencilListUpdateEvent : MikanEvent
	{
	};

}
