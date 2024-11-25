using System;
using System.Runtime.InteropServices;
using System.Text;

namespace MikanXR
{
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

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Initialize(
			MikanLogLevel min_log_level, 
			NativeLogCallback log_callback, 
			out IntPtr out_context);

		[DllImport("MikanCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_SetClientInfo(
			IntPtr context,
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string clientInfo);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetCoreSDKVersion();

		[DllImport("MikanCore.dll")]
		public static extern IntPtr Mikan_GetClientUniqueID(IntPtr context);
		public static string GetClientUniqueID(IntPtr context)
		{
			return Marshal.PtrToStringAnsi(Mikan_GetClientUniqueID(context));
		}

		[DllImport("MikanCore.dll")]
		public static extern bool Mikan_GetIsInitialized(IntPtr context);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetGraphicsDeviceInterface(
			IntPtr context,
			MikanClientGraphicsApi api, 
			IntPtr graphicsDeviceInterface);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetGraphicsDeviceInterface(
			IntPtr context,
			MikanClientGraphicsApi api, 
			out IntPtr outGraphicsDeviceInterface);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_GetRenderTargetDescriptor(
			IntPtr context,
			out MikanRenderTargetDescriptor out_descriptor);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_AllocateRenderTargetTextures(
			IntPtr context,
			ref MikanRenderTargetDescriptor descriptor);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_FreeRenderTargetTextures(IntPtr context);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_WriteColorRenderTargetTexture(
			IntPtr context,
			IntPtr api_color_texture_ptr);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_WriteDepthRenderTargetTexture(
			IntPtr context,
			IntPtr api_depth_texture_ptr, 
			float zNear, 
			float zFar);

		[DllImport("MikanCore.dll")]
		public static extern IntPtr Mikan_GetPackDepthTextureResourcePtr(IntPtr context);

		[DllImport("MikanCore.dll", CharSet = CharSet.Ansi)]
		public static extern int Mikan_Connect(
			IntPtr context,
			string host, 
			string port,
			string connectionRequestJson);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Disconnect(IntPtr context);

		[DllImport("MikanCore.dll")]
		public static extern bool Mikan_GetIsConnected(IntPtr context);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_FetchNextEvent(
			IntPtr context,
			UIntPtr utf8_buffer_size,
			[MarshalAs(UnmanagedType.LPUTF8Str)] 
			StringBuilder out_utf8_buffer,
			out UIntPtr out_utf8_bytes_written);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SendRequestJSON(
			IntPtr context,
			[MarshalAs(UnmanagedType.LPUTF8Str)]
			string utf8_request_json);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetTextResponseCallback(
			IntPtr context,
			NativeTextResponseCallback callback, 
			IntPtr callback_userdata);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_SetBinaryResponseCallback(
			IntPtr context,
			NativeBinaryResponseCallback callback,
			IntPtr callback_userdata);

		[DllImport("MikanCore.dll")]
		public static extern int Mikan_Shutdown(IntPtr context);
	}
}