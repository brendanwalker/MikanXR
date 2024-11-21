// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class WriteColorRenderTargetTexture : MikanRequest
	{
		public static new readonly ulong classId= 16651805590364640671;

		public IntPtr apiColorTexturePtr;
	};

	public class WriteDepthRenderTargetTexture : MikanRequest
	{
		public static new readonly ulong classId= 11933511776700847513;

		public IntPtr apiDepthTexturePtr;
		public float zNear;
		public float zFar;
	};

	public class AllocateRenderTargetTextures : MikanRequest
	{
		public static new readonly ulong classId= 15169388998888474729;

		public MikanRenderTargetDescriptor descriptor;
	};

	public class PublishRenderTargetTextures : MikanRequest
	{
		public static new readonly ulong classId= 1665533327486038831;

		public ulong frameIndex;
	};

	public class FreeRenderTargetTextures : MikanRequest
	{
		public static new readonly ulong classId= 16906743053326766964;

	};

}
