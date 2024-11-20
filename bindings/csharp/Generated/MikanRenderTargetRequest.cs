// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public class WriteColorRenderTargetTexture : MikanRequest
	{
		public IntPtr apiColorTexturePtr;
	};

	public class WriteDepthRenderTargetTexture : MikanRequest
	{
		public IntPtr apiDepthTexturePtr;
		public float zNear;
		public float zFar;
	};

	public class AllocateRenderTargetTextures : MikanRequest
	{
		public MikanRenderTargetDescriptor descriptor;
	};

	public class PublishRenderTargetTextures : MikanRequest
	{
		public ulong frameIndex;
	};

	public class FreeRenderTargetTextures : MikanRequest
	{
	};

}
