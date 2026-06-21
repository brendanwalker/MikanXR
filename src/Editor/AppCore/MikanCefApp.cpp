#include "MikanCefApp.h"

void MikanCefApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	// Replace real webcam/microphone hardware with fake devices in every CEF
	// process.  MikanXR manages camera capture itself through the native video
	// stack; CEF enumerating real webcams generates a stream of non-fatal but
	// noisy errors (IAMCameraControl / IAMVideoProcAmp "No such interface
	// supported") because many cameras do not implement those optional
	// DirectShow interfaces.
	command_line->AppendSwitch("use-fake-device-for-media-stream");

#ifdef WIN32
	// Enable the Windows platform HEVC (H.265) decoder on Windows 10+.
	// The open-source CEF binary is built without proprietary codecs
	// (ffmpeg_branding=Chromium), so H.264/AAC are not available via FFmpeg.
	// Platform HEVC decoding uses the OS codec pack and does not require a
	// proprietary CEF build.
	command_line->AppendSwitchWithValue("enable-features", "PlatformHEVCDecoderSupport,D3D11VideoDecoder");
#endif
}
