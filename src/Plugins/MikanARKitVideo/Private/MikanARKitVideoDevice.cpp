#include "MikanARKitVideoDevice.h"
#include "ARKitRTPHeaderExtension.h"
#include "Cuda/CudaErrorHandling.h"
#include "IMkTexture.h"
#include "MikanARKitVideoDeviceManager.h"
#include "Logger.h"

// gst/cuda/gstcudamemory.h transitively includes <cudaGL.h>/<d3d11.h>/<dxgi.h>
// (via cuda-gst.h) for CUDA-GL/D3D11 interop declarations this file never calls -
// none of that is needed here (Track D owns actual GL interop), but the headers
// still get pulled in. Windows' own <GL/gl.h> (reached via cudaGL.h) expects
// WINGDIAPI/APIENTRY to already be defined consistently by <windows.h>, so
// <windows.h> must be included first or gl.h's declarations fail to parse.
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <gst/app/gstappsink.h>
#include <gst/cuda/gstcudamemory.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include <chrono>
#include <cstring>
#include <sstream>
#include <thread>

// -- GStreamerImpl -----
// Holds the RTP/H.264 receive pipeline for the video channel (basePort+0):
//   udpsrc ! rtpjitterbuffer ! rtph264depay ! h264parse ! nvh264dec
//   ! video/x-raw(memory:CUDAMemory) ! appsink
// Hardware decode via nvh264dec straight to CUDA device memory (ticket C4) - no
// software-decode fallback per the locked-in design decision (see the plan's
// Track C4 edge cases): if nvh264dec/the nvcodec plugin isn't available,
// openOnThread() fails cleanly rather than silently falling back to decodebin's
// software path used by ticket C2's interim pipeline. Kept as an opaque struct
// (rather than members directly on MikanARKitVideoDevice) so GStreamer headers
// don't leak into this plugin's public header, matching
// MikanGStreamerVideoDevice.h's m_impl pattern.
struct GStreamerImpl
{
	uint16_t videoPort;

	GstElement* pipeline= nullptr;
	GstElement* depay= nullptr;
	GstElement* appsink= nullptr;
	GstBus* bus= nullptr;

	explicit GStreamerImpl(uint16_t inVideoPort)
		: videoPort(inVideoPort)
	{
	}

	std::string buildPipelineString() const
	{
		std::stringstream ss;
		ss << "udpsrc port=" << videoPort << " "
		   << "caps=\"application/x-rtp,media=video,encoding-name=H264,payload=96\" "
		   << "! rtpjitterbuffer latency=50 "
		   << "! rtph264depay name=depay "
		   << "! h264parse "
		   << "! nvh264dec "
		   << "! video/x-raw(memory:CUDAMemory) "
		   << "! appsink name=sink";
		return ss.str();
	}

	// Attaches ARKitRTPHeaderExtension (ticket C3) to the depayloader so the
	// frameSeq/captureTimestampUs carried in each RTP packet's header extension
	// (RFC 5285, extension id kARKitRTPHeaderExtensionId) is extracted and attached
	// as ARKitFrameSeqMeta onto rtph264depay's output buffers.
	bool attachFrameSeqExtension() const
	{
		GstRTPHeaderExtension* ext= arkit_rtp_header_extension_new();
		if (ext == nullptr)
			return false;

		gst_rtp_header_extension_set_id(ext, kARKitRTPHeaderExtensionId);

		g_signal_emit_by_name(depay, "add-extension", ext);
		gst_object_unref(ext);

		return true;
	}

	static bool extractFrameDimensions(GstCaps* caps, int& outWidth, int& outHeight)
	{
		outWidth= 0;
		outHeight= 0;

		if (caps == nullptr || gst_caps_is_empty(caps))
			return false;

		GstStructure* structure= gst_caps_get_structure(caps, 0);
		if (structure == nullptr)
			return false;

		const bool bHasWidth= gst_structure_get_int(structure, "width", &outWidth) != FALSE;
		const bool bHasHeight= gst_structure_get_int(structure, "height", &outHeight) != FALSE;

		return bHasWidth && bHasHeight && outWidth > 0 && outHeight > 0;
	}

	static gboolean busCallback(GstBus*, GstMessage* msg, gpointer userData)
	{
		auto* videoDevice= reinterpret_cast<MikanARKitVideoDevice*>(userData);

		switch (GST_MESSAGE_TYPE(msg))
		{
		case GST_MESSAGE_EOS:
		{
			MIKAN_LOG_INFO("MikanARKitVideoDevice::busCallback") << "End of video stream";
			videoDevice->close();
		}
		break;
		case GST_MESSAGE_ERROR:
		{
			GError* error= nullptr;
			gchar* debugInfo= nullptr;
			gst_message_parse_error(msg, &error, &debugInfo);

			MIKAN_LOG_ERROR("MikanARKitVideoDevice::busCallback")
				<< "Error from element " << GST_OBJECT_NAME(msg->src) << ": "
				<< (error != nullptr ? error->message : "unknown");
			MIKAN_LOG_ERROR("MikanARKitVideoDevice::busCallback")
				<< "Debug info: " << (debugInfo != nullptr ? debugInfo : "none");

			g_clear_error(&error);
			g_free(debugInfo);

			videoDevice->close();
		}
		break;
		default:
			break;
		}

		return TRUE;
	}
};

MikanARKitVideoDevice::MikanARKitVideoDevice(MikanARKitVideoDeviceManager* ownerDeviceManager,
											 const ARKitVideoConnectionSettings& connectionInfo)
	: m_ownerDeviceManager(ownerDeviceManager)
	, m_connectionInfo(connectionInfo)
	, m_devicePath("arkit://" + std::to_string(connectionInfo.basePort))
	, m_impl(new GStreamerImpl(connectionInfo.basePort))
	, m_frameCorrelator(connectionInfo.depthStreamingEnabled)
{
	m_frameCorrelator.setBundleCallback([this](ARKitFrameBundle bundle) { notifyFrameBundleReceived(bundle); });

	m_depthReceiver.setFrameCallback(
		[this](ARKitDepthFrame frame)
		{
			// Stash a copy for updateColorAndDepthTextures() (ticket E3) before
			// handing the original off to the correlator (notifyDepthArrived takes
			// it by value/move, so this must copy, not move, first).
			{
				std::lock_guard<std::mutex> lock(m_latestDepthMutex);
				m_latestDepthFrame= frame;
			}
			m_frameCorrelator.notifyDepthArrived(std::move(frame));
		});

	m_poseReceiver.setFrameCallback([this](ARKitPoseFrame frame)
									{ m_frameCorrelator.notifyPoseArrived(std::move(frame)); });
}

MikanARKitVideoDevice::~MikanARKitVideoDevice()
{
	close();
	delete m_impl;
}

// -- Device Listener -----
void MikanARKitVideoDevice::addListener(IARKitVideoDeviceListener* listener)
{
	std::lock_guard<std::mutex> lock(m_listenersMutex);
	m_listeners.insert(listener);
}

void MikanARKitVideoDevice::removeListener(IARKitVideoDeviceListener* listener)
{
	std::lock_guard<std::mutex> lock(m_listenersMutex);
	m_listeners.erase(listener);
}

// -- Device Properties -----
const char* MikanARKitVideoDevice::getDevicePath() const { return m_devicePath.c_str(); }

const char* MikanARKitVideoDevice::getFriendlyName() const { return getDevicePath(); }

// -- Device Activation -----
eVideoOpeningStatus MikanARKitVideoDevice::open()
{
	if (m_openState == eOpenState::open || m_openState == eOpenState::opening)
		return getVideoOpeningStatus();

	// Depth/pose sockets bind quickly (a single non-blocking recvfrom-socket bind
	// call each - see UdpReceiveSocket::open), unlike GStreamer pipeline
	// construction (which genuinely needs backgrounding - see openOnThread()), so
	// start them synchronously here rather than deferring to the background thread.
	if (m_connectionInfo.depthStreamingEnabled)
	{
		const uint16_t depthPort= static_cast<uint16_t>(m_connectionInfo.basePort + 1);
		if (!m_depthReceiver.start(depthPort))
		{
			MIKAN_LOG_ERROR("MikanARKitVideoDevice::open") << "Failed to start depth receiver on port " << depthPort;
			m_openState= eOpenState::failed;
			return eVideoOpeningStatus::failed;
		}
	}

	const uint16_t posePort= static_cast<uint16_t>(m_connectionInfo.basePort + 2);
	if (!m_poseReceiver.start(posePort))
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::open") << "Failed to start pose receiver on port " << posePort;
		m_depthReceiver.stop();
		m_openState= eOpenState::failed;
		return eVideoOpeningStatus::failed;
	}

	// Launch the GStreamer video pipeline setup on a background thread, matching
	// MikanGStreamerVideoDevice's rationale (pipeline construction is slow on first
	// call - loads GStreamer DLLs).
	m_openState= eOpenState::opening;
	auto promise= std::make_shared<std::promise<bool>>();
	m_openFuture= promise->get_future();
	std::thread([promise, this]() mutable { promise->set_value(openOnThread()); }).detach();

	return eVideoOpeningStatus::opening;
}

bool MikanARKitVideoDevice::openOnThread()
{
	const std::string pipelineString= m_impl->buildPipelineString();

	// nvh264dec is a hard dependency (v1 has no software-decode fallback per the
	// locked-in design decision) - if the nvcodec plugin isn't installed, or an
	// NVIDIA GPU/driver isn't present, gst_parse_launch fails synchronously here
	// with a "no element \"nvh264dec\"" GError (an explicit element name, unlike
	// ticket C2's decodebin, is resolved immediately rather than lazily on first
	// buffer), which is logged below and surfaced as a device-open failure.
	GError* error= nullptr;
	m_impl->pipeline= gst_parse_launch(pipelineString.c_str(), &error);
	if (error != nullptr)
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::openOnThread")
			<< "Failed to create pipeline (is an NVIDIA GPU/driver and the GStreamer nvcodec plugin available?): "
			<< error->message;
		g_error_free(error);
		return false;
	}

	m_impl->appsink= gst_bin_get_by_name(GST_BIN(m_impl->pipeline), "sink");
	if (m_impl->appsink == nullptr)
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::openOnThread") << "Failed to find appsink in pipeline";
		return false;
	}

	m_impl->depay= gst_bin_get_by_name(GST_BIN(m_impl->pipeline), "depay");
	if (m_impl->depay == nullptr)
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::openOnThread") << "Failed to find rtph264depay in pipeline";
		return false;
	}

	if (!m_impl->attachFrameSeqExtension())
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::openOnThread")
			<< "Failed to attach ARKit frameSeq RTP header extension";
		return false;
	}

	m_impl->bus= gst_pipeline_get_bus(GST_PIPELINE(m_impl->pipeline));
	if (m_impl->bus == nullptr)
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::openOnThread") << "Failed to get pipeline bus";
		return false;
	}

	return true;
}

eVideoOpeningStatus MikanARKitVideoDevice::getVideoOpeningStatus() const
{
	switch (m_openState)
	{
	case eOpenState::open:
		return eVideoOpeningStatus::open;
	case eOpenState::opening:
		return eVideoOpeningStatus::opening;
	case eOpenState::failed:
		return eVideoOpeningStatus::failed;
	default:
		return eVideoOpeningStatus::closed;
	}
}

void MikanARKitVideoDevice::update(float deltaSeconds)
{
	// Poll the async open future
	if (m_openState == eOpenState::opening)
	{
		if (m_openFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			const bool bSuccess= m_openFuture.get();
			if (bSuccess)
			{
				// Note: NOT gst_bus_add_watch() here - that attaches a GSource to the
				// default GLib main context, but nothing in this codebase ever pumps
				// that context (no g_main_loop_run/g_main_context_iteration anywhere),
				// so a watch-based callback would simply never fire. Bus messages are
				// instead polled explicitly each tick below, alongside the appsink
				// pull.
				gst_app_sink_set_emit_signals(GST_APP_SINK(m_impl->appsink), FALSE);
				gst_app_sink_set_drop(GST_APP_SINK(m_impl->appsink), TRUE);
				gst_app_sink_set_max_buffers(GST_APP_SINK(m_impl->appsink), 1);

				m_openState= eOpenState::open;

				{
					std::lock_guard<std::mutex> lock(m_listenersMutex);
					for (IARKitVideoDeviceListener* listener : m_listeners)
						listener->notifyDeviceOpened(this);
				}

				if (m_pendingStreamStartAfterOpen)
				{
					m_pendingStreamStartAfterOpen= false;
					startVideoStream();
				}
			}
			else
			{
				m_openState= eOpenState::failed;
				m_depthReceiver.stop();
				m_poseReceiver.stop();
			}
		}
		return;
	}

	if (m_openState != eOpenState::open)
		return;

	// ARKitDepthReceiver/ARKitPoseReceiver each drive their own worker thread
	// already; the correlator is the one piece that needs an explicit periodic
	// tick to evict stale (incomplete) pending frame bundles - see
	// ARKitFrameCorrelator::sweepStaleFrames.
	m_frameCorrelator.sweepStaleFrames(std::chrono::steady_clock::now());

	// Non-blocking bus drain (see the note in the open-completion branch above for
	// why this is polled explicitly rather than via gst_bus_add_watch).
	while (GstMessage* msg=
			   gst_bus_pop_filtered(m_impl->bus, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)))
	{
		GStreamerImpl::busCallback(m_impl->bus, msg, this);
		gst_message_unref(msg);

		if (m_openState != eOpenState::open)
			return; // busCallback triggered close()
	}

	if (m_streamingStatus != eVideoStreamingStatus::started && m_streamingStatus != eVideoStreamingStatus::pendingStart)
	{
		return;
	}

	GstSample* sample= gst_app_sink_try_pull_sample(GST_APP_SINK(m_impl->appsink), 0);
	if (sample != nullptr)
	{
		m_timeSinceLastFrameSeconds= 0.0f;

		GstBuffer* buffer= gst_sample_get_buffer(sample);
		GstCaps* caps= gst_sample_get_caps(sample);

		int width= 0, height= 0;
		if (buffer != nullptr && GStreamerImpl::extractFrameDimensions(caps, width, height))
		{
			if (m_streamingStatus == eVideoStreamingStatus::pendingStart)
			{
				MIKAN_LOG_INFO("MikanARKitVideoDevice::update")
					<< "Video stream started (" << width << "x" << height << ")";
				m_streamingStatus= eVideoStreamingStatus::started;
			}

			// Attached by ARKitRTPHeaderExtension (ticket C3) at the rtph264depay
			// stage, from the RTP header extension carried on the encoded packets;
			// must survive h264parse -> nvh264dec's internal buffer transforms to
			// still be present here (see ARKitFrameSeqMeta's transform function).
			const ARKitFrameSeqMeta* frameSeqMeta= arkit_buffer_get_frame_seq_meta(buffer);

			// GST_MAP_CUDA maps the buffer's device pointer itself rather than
			// downloading to a host-readable copy (ticket C4) - map.data is a
			// CUdeviceptr value, not CPU-readable memory; it must never be
			// dereferenced from the CPU, only handed to CUDA APIs (see Track D's
			// JBU kernel, which will consume this as its guide image).
			GstMapInfo map;
			if (gst_buffer_map(buffer, &map, static_cast<GstMapFlags>(GST_MAP_READ | GST_MAP_CUDA)))
			{
				const CUdeviceptr devicePtr= reinterpret_cast<CUdeviceptr>(map.data);

				// LoggerStream has no std::hex/std::dec manipulator support (unlike
				// std::ostream) - its operator<<(const void*) already formats as a
				// hex address, so log the pointer value that way instead.
				const void* devicePtrForLogging= reinterpret_cast<const void*>(devicePtr);

				if (frameSeqMeta != nullptr)
				{
					MIKAN_LOG_INFO("MikanARKitVideoDevice::update")
						<< "Decoded CUDA video frame: " << width << "x" << height
						<< ", devicePtr=" << devicePtrForLogging << ", frameSeq=" << frameSeqMeta->frameSeq;

					// videoFrameHandle is left null - correlating a stable,
					// DLL-boundary-safe decoded-pixel handle across the bundle is a
					// later ticket's job (see ARKitFrameBundle's own comment); this
					// only establishes that video arrived for this frameSeq, and when.
					m_frameCorrelator.notifyVideoArrived(frameSeqMeta->frameSeq, frameSeqMeta->captureTimestampUs,
														 nullptr);
				}
				else
				{
					// Expected if the sender doesn't attach the extension (e.g. a
					// non-MikanARStreamer RTP source) or if an intervening element
					// dropped the meta - not fed to the correlator since there's no
					// frameSeq to key it by.
					MIKAN_LOG_WARNING("MikanARKitVideoDevice::update")
						<< "Decoded CUDA video frame: " << width << "x" << height
						<< ", devicePtr=" << devicePtrForLogging
						<< ", but no frameSeq RTP header extension meta was present";
				}

				// Zero-copy CUDA-GL texture pipeline (ticket E3): NV12->RGBA display
				// texture + JBU-upsampled depth texture. Runs regardless of whether the
				// frameSeq meta was present above - display shouldn't depend on pose
				// correlation working.
				updateColorAndDepthTextures(buffer, devicePtr, width, height);

				gst_buffer_unmap(buffer, &map);
			}
			else
			{
				MIKAN_LOG_ERROR("MikanARKitVideoDevice::update") << "Failed to map decoded video buffer as CUDA memory";
			}
		}

		gst_sample_unref(sample);
	}
	else if (m_streamingStatus == eVideoStreamingStatus::started)
	{
		m_timeSinceLastFrameSeconds+= deltaSeconds;
		if (m_timeSinceLastFrameSeconds >= k_streamTimeoutSeconds)
		{
			MIKAN_LOG_WARNING("MikanARKitVideoDevice::update")
				<< "No video frame received for " << m_timeSinceLastFrameSeconds << "s - reopening video pipeline";

			m_pendingStreamStartAfterOpen= true;
			close();
			open();
		}
	}
}

void MikanARKitVideoDevice::close()
{
	if (m_openState == eOpenState::opening && m_openFuture.valid())
	{
		// Block until the in-flight async open completes so we can safely clean up
		m_openFuture.get();
		// Don't notify — the device was never fully open from the caller's perspective
	}
	else if (m_openState == eOpenState::open)
	{
		std::lock_guard<std::mutex> lock(m_listenersMutex);
		for (IARKitVideoDeviceListener* listener : m_listeners)
			listener->notifyDeviceClosed(this);

		// Clients need to reregister as listeners when they reopen
		m_listeners.clear();
	}

	m_openState= eOpenState::closed;
	m_streamingStatus= eVideoStreamingStatus::stopped;
	m_timeSinceLastFrameSeconds= 0.0f;

	m_depthReceiver.stop();
	m_poseReceiver.stop();

	// Tear down the whole CUDA-GL texture pipeline on close (ticket E3) rather than
	// trying to keep it alive across reopen - the GStreamer pipeline's own
	// GstCudaContext may not survive a close/reopen cycle intact (see project memory
	// on CUDA context corruption after a fault), so the safest contract is "fully
	// rebuilt on next successful decode", matching how m_impl->pipeline itself is
	// unconditionally torn down and rebuilt below.
	if (m_bTexturePipelineInitialized)
	{
		m_jbuKernel.shutdown();
		m_nv12Kernel.shutdown();
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();

		if (m_guideRgbBuffer != 0)
		{
			cuMemFree(m_guideRgbBuffer);
			m_guideRgbBuffer= 0;
		}
		if (m_depthLowBuffer != 0)
		{
			cuMemFree(m_depthLowBuffer);
			m_depthLowBuffer= 0;
		}
		if (m_confidenceLowBuffer != 0)
		{
			cuMemFree(m_confidenceLowBuffer);
			m_confidenceLowBuffer= 0;
		}

		m_bTexturePipelineInitialized= false;
		m_textureWidth= 0;
		m_textureHeight= 0;
	}

	// Independent of m_bTexturePipelineInitialized - ensureCudaContext() can
	// succeed (creating m_cudaContext) even if a later texture-pipeline init step
	// fails, so this must not be skipped just because the pipeline never finished
	// initializing.
	if (m_cudaContext != nullptr)
	{
		cuCtxDestroy(m_cudaContext);
		m_cudaContext= nullptr;
	}

	if (m_impl->pipeline != nullptr)
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
	}

	if (m_impl->bus != nullptr)
	{
		gst_object_unref(m_impl->bus);
		m_impl->bus= nullptr;
	}

	if (m_impl->appsink != nullptr)
	{
		gst_object_unref(m_impl->appsink);
		m_impl->appsink= nullptr;
	}

	if (m_impl->depay != nullptr)
	{
		gst_object_unref(m_impl->depay);
		m_impl->depay= nullptr;
	}

	if (m_impl->pipeline != nullptr)
	{
		gst_object_unref(m_impl->pipeline);
		m_impl->pipeline= nullptr;
	}
}

// -- Zero-copy CUDA-GL texture pipeline (ticket E3) -----
bool MikanARKitVideoDevice::ensureCudaContext()
{
	if (m_cudaContext != nullptr)
	{
		// Re-assert as current every call (cheap no-op if already current) - see
		// this method's declaration comment for why this can't be a one-time thing.
		return checkCudaResult(cuCtxSetCurrent(m_cudaContext), "cuCtxSetCurrent");
	}

	if (!checkCudaResult(cuInit(0), "cuInit"))
		return false;

	CUdevice device;
	if (!checkCudaResult(cuDeviceGet(&device, 0), "cuDeviceGet"))
		return false;

	// 4-arg cuCtxCreate (cuCtxCreate_v4, adds a CUctxCreateParams* param as of CUDA
	// 13.1) - nullptr for that param matches every other call site in this codebase
	// (see JBUKernel's own D1-era finding).
	if (!checkCudaResult(cuCtxCreate(&m_cudaContext, nullptr, 0, device), "cuCtxCreate"))
	{
		m_cudaContext= nullptr;
		return false;
	}

	// cuCtxCreate already makes the new context current on this thread, but
	// asserting it explicitly costs nothing and keeps this function's contract
	// simple ("returns true only if m_cudaContext is current on return").
	return checkCudaResult(cuCtxSetCurrent(m_cudaContext), "cuCtxSetCurrent");
}

bool MikanARKitVideoDevice::ensureTexturePipelineInitialized(int width, int height)
{
	if (m_bTexturePipelineInitialized && width == m_textureWidth && height == m_textureHeight)
		return true;

	if (m_bTexturePipelineInitialized)
	{
		// Dimensions changed mid-stream - shouldn't normally happen (ARKit's video
		// resolution is fixed for a session), but resize cleanly rather than
		// reuse/leak stale buffers if it does. Kernel modules themselves aren't
		// size-dependent, so they're left alone.
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();
		if (m_guideRgbBuffer != 0)
		{
			cuMemFree(m_guideRgbBuffer);
			m_guideRgbBuffer= 0;
		}
		m_bTexturePipelineInitialized= false;
	}

	if (!m_colorTexture.init(width, height))
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::ensureTexturePipelineInitialized") << "Failed to init color texture";
		return false;
	}

	if (!m_depthTexture.init(width, height))
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::ensureTexturePipelineInitialized") << "Failed to init depth texture";
		m_colorTexture.shutdown();
		return false;
	}

	if (!m_nv12Kernel.isInitialized() && !m_nv12Kernel.init(NV12_KERNEL_PTX_PATH))
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::ensureTexturePipelineInitialized")
			<< "Failed to init NV12 conversion kernel";
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();
		return false;
	}

	if (!m_jbuKernel.isInitialized() && !m_jbuKernel.init(JBU_KERNEL_PTX_PATH))
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::ensureTexturePipelineInitialized") << "Failed to init JBU kernel";
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();
		return false;
	}

	m_guideStrideBytes= width * 3;
	if (!checkCudaResult(cuMemAlloc(&m_guideRgbBuffer, static_cast<size_t>(m_guideStrideBytes) * height),
						 "cuMemAlloc(guideRgb)"))
	{
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();
		return false;
	}

	if (m_depthLowBuffer == 0
		&& !checkCudaResult(cuMemAlloc(&m_depthLowBuffer, kARKitDepthPixelCount * sizeof(uint16_t)),
							"cuMemAlloc(depthLow)"))
	{
		cuMemFree(m_guideRgbBuffer);
		m_guideRgbBuffer= 0;
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();
		return false;
	}

	if (m_confidenceLowBuffer == 0
		&& !checkCudaResult(cuMemAlloc(&m_confidenceLowBuffer, kARKitDepthPixelCount), "cuMemAlloc(confidenceLow)"))
	{
		cuMemFree(m_guideRgbBuffer);
		m_guideRgbBuffer= 0;
		cuMemFree(m_depthLowBuffer);
		m_depthLowBuffer= 0;
		m_colorTexture.shutdown();
		m_depthTexture.shutdown();
		return false;
	}

	m_textureWidth= width;
	m_textureHeight= height;
	m_bTexturePipelineInitialized= true;
	return true;
}

void MikanARKitVideoDevice::updateColorAndDepthTextures(GstBuffer* buffer, CUdeviceptr devicePtr, int width, int height)
{
	if (!ensureCudaContext())
		return;

	if (!ensureTexturePipelineInitialized(width, height))
		return;

	// NV12 plane layout (Y full-res, UV half-res interleaved) - stride/offset come
	// from the buffer's own GstVideoMeta rather than being assumed, since CUDA
	// memory allocations can be padded/aligned differently than a tightly-packed
	// buffer would be.
	GstVideoMeta* videoMeta= gst_buffer_get_video_meta(buffer);
	if (videoMeta == nullptr || videoMeta->n_planes < 2)
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::updateColorAndDepthTextures")
			<< "Missing/invalid GstVideoMeta on decoded NV12 buffer";
		return;
	}

	const CUdeviceptr yPlane= devicePtr + videoMeta->offset[0];
	const int yStrideBytes= static_cast<int>(videoMeta->stride[0]);
	const CUdeviceptr uvPlane= devicePtr + videoMeta->offset[1];
	const int uvStrideBytes= static_cast<int>(videoMeta->stride[1]);

	// -- Color: NV12 -> RGBA (display) + RGB (JBU guide) --
	CUsurfObject colorSurface= 0;
	if (m_colorTexture.beginCudaAccess(colorSurface))
	{
		m_nv12Kernel.convert(yPlane, yStrideBytes, uvPlane, uvStrideBytes, width, height, colorSurface,
							 m_guideRgbBuffer, m_guideStrideBytes);
		// Block until the conversion completes before unmapping the color texture
		// (endCudaAccess's stream param is for synchronizing the unmap against
		// in-flight work on that same stream, which NV12ConversionKernel doesn't
		// expose - synchronizing here is simpler and keeps the guide-RGB buffer
		// write-complete guarantee below just as valid) - see NV12ConversionKernel.h.
		m_nv12Kernel.synchronize();
		m_colorTexture.endCudaAccess(nullptr);
	}

	// -- Depth: latest cached ARKit depth+confidence, JBU-upsampled against the
	// guide-RGB buffer the color pass just wrote --
	ARKitDepthFrame latestDepth;
	bool bHasDepth= false;
	{
		std::lock_guard<std::mutex> lock(m_latestDepthMutex);
		if (m_latestDepthFrame.has_value())
		{
			latestDepth= *m_latestDepthFrame;
			bHasDepth= true;
		}
	}

	if (bHasDepth)
	{
		checkCudaResult(
			cuMemcpyHtoD(m_depthLowBuffer, latestDepth.depthMM.data(), latestDepth.depthMM.size() * sizeof(uint16_t)),
			"cuMemcpyHtoD(depthLow)");
		checkCudaResult(
			cuMemcpyHtoD(m_confidenceLowBuffer, latestDepth.confidence.data(), latestDepth.confidence.size()),
			"cuMemcpyHtoD(confidenceLow)");

		CUsurfObject depthSurface= 0;
		if (m_depthTexture.beginCudaAccess(depthSurface))
		{
			// D5-tuned defaults (see JBUParams in JBUKernel.h) - not yet wired to
			// ARKitVideoSourceDefinition's own JBU tuning properties (stored since
			// ticket E1 but not consumed by anything until now); a reasonable
			// follow-up, not required for this ticket's zero-copy display goal.
			JBUParams params;
			m_jbuKernel.upsampleToSurface(m_depthLowBuffer, kARKitDepthWidth, kARKitDepthHeight,
										  kARKitDepthWidth * static_cast<int>(sizeof(uint16_t)), m_confidenceLowBuffer,
										  kARKitDepthWidth, m_guideRgbBuffer, width, height, m_guideStrideBytes,
										  depthSurface, width, height, params);
			m_jbuKernel.synchronize();
			m_depthTexture.endCudaAccess(nullptr);
		}
	}
}

uint32_t MikanARKitVideoDevice::getColorTextureGlId() const
{
	IMkTexturePtr texture= m_colorTexture.getTexture();
	return texture != nullptr ? texture->getGlTextureId() : 0;
}

uint32_t MikanARKitVideoDevice::getDepthTextureGlId() const
{
	IMkTexturePtr texture= m_depthTexture.getTexture();
	return texture != nullptr ? texture->getGlTextureId() : 0;
}

// -- Video Settings -----
bool MikanARKitVideoDevice::isVideoSettingSupported(const eVideoSettingType) const { return false; }

bool MikanARKitVideoDevice::getVideoSettingConstraint(const eVideoSettingType, VideoSettingConstraint&) const
{
	return false;
}

void MikanARKitVideoDevice::setVideoSetting(const eVideoSettingType, int)
{
	// Not supported
}

int MikanARKitVideoDevice::getVideoSetting(const eVideoSettingType) const { return -1; }

// -- Video Streaming -----
eVideoStreamingStatus MikanARKitVideoDevice::startVideoStream()
{
	if (m_openState == eOpenState::open)
	{
		if (m_streamingStatus == eVideoStreamingStatus::stopped)
		{
			m_timeSinceLastFrameSeconds= 0.0f;
			gst_element_set_state(m_impl->pipeline, GST_STATE_PLAYING);
			m_streamingStatus= eVideoStreamingStatus::pendingStart;
		}

		return m_streamingStatus;
	}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus MikanARKitVideoDevice::getVideoStreamingStatus() const { return m_streamingStatus; }

void MikanARKitVideoDevice::stopVideoStream()
{
	if (m_openState == eOpenState::open && m_impl->pipeline != nullptr)
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_PAUSED);
	}

	m_streamingStatus= eVideoStreamingStatus::stopped;
	m_pendingStreamStartAfterOpen= false;
}

// -- Frame bundle conversion -----
void MikanARKitVideoDevice::notifyFrameBundleReceived(const ARKitFrameBundle& internalBundle)
{
	ARKitVideoFrameBundle publicBundle;
	publicBundle.frameSeq= internalBundle.frameSeq;
	publicBundle.timestampUs= internalBundle.timestampUs;

	publicBundle.hasVideo= internalBundle.hasVideo;
	// internalBundle.videoFrameHandle is intentionally left unused here - ticket C3
	// only established frameSeq/timestamp correlation for video arrival (see
	// MikanARKitVideoDevice::update's appsink pull site); handing decoded pixel data
	// across the DLL boundary through this bundle is a later ticket's job.
	publicBundle.videoData= nullptr;
	publicBundle.videoWidth= 0;
	publicBundle.videoHeight= 0;

	if (internalBundle.depth.has_value())
	{
		publicBundle.hasDepth= true;
		publicBundle.depth.width= kARKitDepthWidth;
		publicBundle.depth.height= kARKitDepthHeight;
		publicBundle.depth.depthMM= internalBundle.depth->depthMM.data();
		publicBundle.depth.confidence= internalBundle.depth->confidence.data();
	}

	if (internalBundle.pose.has_value())
	{
		publicBundle.hasPose= true;
		std::memcpy(publicBundle.pose.transform, internalBundle.pose->transform, sizeof(publicBundle.pose.transform));
		publicBundle.pose.fx= internalBundle.pose->fx;
		publicBundle.pose.fy= internalBundle.pose->fy;
		publicBundle.pose.cx= internalBundle.pose->cx;
		publicBundle.pose.cy= internalBundle.pose->cy;
		publicBundle.pose.imageWidth= internalBundle.pose->imageWidth;
		publicBundle.pose.imageHeight= internalBundle.pose->imageHeight;
	}

	std::lock_guard<std::mutex> lock(m_listenersMutex);
	for (IARKitVideoDeviceListener* listener : m_listeners)
		listener->notifyFrameBundleReceived(publicBundle);
}
