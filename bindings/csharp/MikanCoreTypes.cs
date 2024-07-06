using System.Runtime.InteropServices;

namespace MikanXR
{
	[StructLayout(LayoutKind.Sequential)]
	public struct MikanRenderTargetDescriptor
	{
		public MikanColorBufferType color_buffer_type;
		public MikanDepthBufferType depth_buffer_type;
		public uint width;
		public uint height;
		public MikanClientGraphicsApi graphicsAPI;
	};

	[StructLayout(LayoutKind.Sequential)]
	public struct MikanClientFrameRendered
	{
		public ulong frame_index;
	};
}