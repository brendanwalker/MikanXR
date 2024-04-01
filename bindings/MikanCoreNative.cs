using System.Collections;
using System.Runtime.InteropServices;
using System.Buffers;

namespace MikanXR
{
	protected class MikanCoreNative
	{
		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeLogCallback(
			int log_level,
			[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPStr)]
			string log_message);

		[global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
		public delegate void NativeResponseCallback(
			int request_id,
			[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPUTF8Str)]
			string utf8_response_string,
			IntPtr userdata);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Initialize(MikanLogLevel min_log_level, NativeLogCallback log_callback);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetCoreSDKVersion();

		[DllImport("MikanCore.dll")]
		public static extern bool Mikan_GetIsInitialized();

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetGraphicsDeviceInterface(
			MikanClientGraphicsApi api, 
			IntPtr graphicsDeviceInterface);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetGraphicsDeviceInterface(
			MikanClientGraphicsApi api, 
			out IntPtr outGraphicsDeviceInterface);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_AllocateRenderTargetBuffers(
			ref MikanRenderTargetDescriptor descriptor, 
			out int out_request_id);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_PublishRenderTargetTexture(
			IntPtr ApiTexturePtr, 
			ref MikanClientFrameRendered frame_info);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_FreeRenderTargetBuffers(out int out_request_id);

		[DllImport("MikanCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_SetClientProperty(
			string key, 
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string value);

		[DllImport("MikanCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_Connect(string host, string port);

		[DllImport("MikanCore.dll")]
		public static extern bool Mikan_GetIsConnected();

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_FetchNextEvent(
			UIntPtr utf8_buffer_size,
			[MarshalAs(UnmanagedType.LPUTF8Str)] 
			StringBuilder out_utf8_buffer,
			out UIntPtr out_utf8_bytes_written);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SendRequest(
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string utf8_request_name, 
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string utf8_payload,
			int request_version,
			out int out_request_id);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetResponseCallback(
			NativeResponseCallback callback, 
			IntPtr callback_userdata);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Disconnect();

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Shutdown();
	}
}