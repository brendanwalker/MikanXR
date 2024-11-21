// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class MikanVideoSourceOpenedEvent : MikanEvent
	{
		public static new readonly ulong classId= 5034499706629648632;

	};

	public class MikanVideoSourceClosedEvent : MikanEvent
	{
		public static new readonly ulong classId= 12980885911741629231;

	};

	public class MikanVideoSourceNewFrameEvent : MikanEvent
	{
		public static new readonly ulong classId= 1235622184979266446;

		public MikanVector3f cameraForward;
		public MikanVector3f cameraUp;
		public MikanVector3f cameraPosition;
		public ulong frame;
	};

	public class MikanVideoSourceAttachmentChangedEvent : MikanEvent
	{
		public static new readonly ulong classId= 17057501295645036020;

	};

	public class MikanVideoSourceIntrinsicsChangedEvent : MikanEvent
	{
		public static new readonly ulong classId= 14058521249612088325;

	};

	public class MikanVideoSourceModeChangedEvent : MikanEvent
	{
		public static new readonly ulong classId= 2985204106626338658;

	};

}
