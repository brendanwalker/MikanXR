// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanQuadStencilListUpdateEvent : MikanEvent
	{
		public static new readonly ulong classId= 17403027254738745545;

	};

	public class MikanModelStencilListUpdateEvent : MikanEvent
	{
		public static new readonly ulong classId= 5443530884967588781;

	};

	public class MikanStencilNameUpdateEvent : MikanEvent
	{
		public static new readonly ulong classId= 5445856050236674031;

		public int stencil_id;
		public string stencil_name;
	};

	public class MikanStencilPoseUpdateEvent : MikanEvent
	{
		public static new readonly ulong classId= 11891467771289033533;

		public MikanTransform transform;
		public int stencil_id;
	};

	public class MikanBoxStencilListUpdateEvent : MikanEvent
	{
		public static new readonly ulong classId= 16227253234774949733;

	};

}
