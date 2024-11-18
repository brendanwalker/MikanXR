// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Structs
	public struct WriteColorRenderTargetTexture : MikanRequest
	{
		public IntPtr apiColorTexturePtr { get; set; }
	};

	public struct WriteDepthRenderTargetTexture : MikanRequest
	{
		public IntPtr apiDepthTexturePtr { get; set; }
		public float zNear { get; set; }
		public float zFar { get; set; }
	};

	public struct AllocateRenderTargetTextures : MikanRequest
	{
		public MikanRenderTargetDescriptor descriptor { get; set; }
	};

	public struct PublishRenderTargetTextures : MikanRequest
	{
		public ulong frameIndex { get; set; }
	};

	public struct FreeRenderTargetTextures : MikanRequest
	{
	};

}
