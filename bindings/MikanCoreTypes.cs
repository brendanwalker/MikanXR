using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;

namespace MikanXR
{
	[StructLayout(LayoutKind.Sequential)]
	public struct MikanRenderTargetDescriptor
	{
		MikanColorBufferType color_buffer_type;
		MikanDepthBufferType depth_buffer_type;
		uint width;
		uint height;
		MikanClientGraphicsApi graphicsAPI;
	};

	[StructLayout(LayoutKind.Sequential)]
	struct MikanClientFrameRendered
	{
		ulong frame_index;
	};
}