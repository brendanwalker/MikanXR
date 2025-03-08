using System;
using System.Runtime.InteropServices;
using System.Text;

namespace MikanXR
{
	[StructLayout(LayoutKind.Sequential)]
	public struct MikanRenderTargetDescriptor_Native
	{
		public MikanColorBufferType color_buffer_type;
		public MikanDepthBufferType depth_buffer_type;
		public uint width;
		public uint height;
		public MikanClientGraphicsApi graphicsAPI;
	};

	public class MikanCoreNative
	{
		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeLogCallback(
			int log_level,
			[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPStr)]
			string log_message);

		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeTextResponseCallback(
			int request_id,
			[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPUTF8Str)]
			string utf8_response_string,
			IntPtr userdata);

		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeBinaryResponseCallback(
			IntPtr buffer,
			UIntPtr buffer_size,
			IntPtr userdata);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_Initialize(
			MikanLogLevel min_log_level, 
			NativeLogCallback log_callback, 
			out IntPtr out_context);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_GetClientAPIVersion();

		[DllImport("MikanClientCore.dll")]
		public static extern IntPtr Mikan_GetClientUniqueID(IntPtr context);
		public static string GetClientUniqueID(IntPtr context)
		{
			return Marshal.PtrToStringAnsi(Mikan_GetClientUniqueID(context));
		}

		[DllImport("MikanClientCore.dll")]
		public static extern bool Mikan_GetIsInitialized(IntPtr context);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_SetGraphicsDeviceInterface(
			IntPtr context,
			MikanClientGraphicsApi api, 
			IntPtr graphicsDeviceInterface);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_GetGraphicsDeviceInterface(
			IntPtr context,
			MikanClientGraphicsApi api, 
			out IntPtr outGraphicsDeviceInterface);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_GetRenderTargetDescriptor(
			IntPtr context,
			out MikanRenderTargetDescriptor_Native out_descriptor);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_AllocateRenderTargetTextures(
			IntPtr context,
			ref MikanRenderTargetDescriptor_Native descriptor);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_FreeRenderTargetTextures(IntPtr context);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_WriteColorRenderTargetTexture(
			IntPtr context,
			IntPtr api_color_texture_ptr);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_WriteDepthRenderTargetTexture(
			IntPtr context,
			IntPtr api_depth_texture_ptr, 
			float zNear, 
			float zFar);

		[DllImport("MikanClientCore.dll")]
		public static extern IntPtr Mikan_GetPackDepthTextureResourcePtr(IntPtr context);

		[DllImport("MikanClientCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_Connect(
			IntPtr context,
			string host, 
			string port);

		[DllImport("MikanClientCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_Disconnect(
			IntPtr context,
			UInt16 code,
			string reason);

		[DllImport("MikanClientCore.dll")]
		public static extern bool Mikan_GetIsConnected(IntPtr context);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_FetchNextEvent(
			IntPtr context,
			UIntPtr utf8_buffer_size,
			[MarshalAs(UnmanagedType.LPUTF8Str)] 
			StringBuilder out_utf8_buffer,
			out UIntPtr out_utf8_bytes_written);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_SendRequestJSON(
			IntPtr context,
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string utf8_request_json);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_SetTextResponseCallback(
			IntPtr context,
			NativeTextResponseCallback callback, 
			IntPtr callback_userdata);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_SetBinaryResponseCallback(
			IntPtr context,
			NativeBinaryResponseCallback callback,
			IntPtr callback_userdata);

		[DllImport("MikanClientCore.dll")]
		public static extern int Mikan_Shutdown(IntPtr context);
	}
}