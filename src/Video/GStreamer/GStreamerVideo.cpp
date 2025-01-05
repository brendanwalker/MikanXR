// -- includes -----
#include "GStreamerVideo.h"
#include "DeviceInterface.h"
#include "Logger.h"
#include "GStreamerCameraEnumerator.h"
#include "WorkerThread.h"

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class GStreamerVideoDeviceImpl
{
public:
	GstElement* pipeline= nullptr;
	GstElement* appsink= nullptr;
	GstBus* bus = nullptr;

	GStreamerVideoDeviceImpl() = default;

	static gboolean busCallback(GstBus* bus, GstMessage* msg, gpointer data)
	{
		GStreamerVideoDevice* videoDevice = reinterpret_cast<GStreamerVideoDevice*>(data);

		switch (GST_MESSAGE_TYPE(msg))
		{
			case GST_MESSAGE_EOS:
				{
					MIKAN_LOG_INFO("bus_call") << "End of stream";
					videoDevice->stopVideoStream();
				} break;

			case GST_MESSAGE_ERROR:
				{
					GError* err;
					gchar* debug_info;

					gst_message_parse_error(msg, &err, &debug_info);
					MIKAN_LOG_ERROR("gstreamer::bus_call") << "Error received from element " << GST_OBJECT_NAME(msg->src) << ": " << err->message;
					MIKAN_LOG_ERROR("gstreamer::bus_call") << "Debugging information: " << (debug_info ? debug_info : "none");
					g_clear_error(&err);
					g_free(debug_info);

					videoDevice->close();
				} break;

			default:
				break;
		}

		return TRUE;

	}
};

struct GstVideoFrameInfo
{
	int width= 0;
	int height= 0;
	double framerate= 0;
	std::string bufferFormat;
	std::string modeName;

	GstVideoFrameInfo() = default;
	
	bool isValid() const { 
		return 
		width > 0 && 
		height > 0 && 
		framerate > 0 && 
		!bufferFormat.empty() &&
		!modeName.empty(); 
	}
};

namespace GStreamerUtils
{
	bool extractVideoFrameInfo(GstCaps* caps, GstVideoFrameInfo& outFrameInfo)
	{
		outFrameInfo= GstVideoFrameInfo();

		if (!caps || gst_caps_is_empty(caps))
		{
			return false;
		}

		for (guint i = 0; i < gst_caps_get_size(caps); ++i)
		{
			GstStructure* structure = gst_caps_get_structure(caps, i);

			// Iterate over each field
			//{
			//	int num_fields = gst_structure_n_fields(structure);
			//	for (int i = 0; i < num_fields; ++i)
			//	{
			//		// Get the field name
			//		const gchar* field_name = gst_structure_nth_field_name(structure, i);
			//		MIKAN_LOG_INFO("extractVideoFrameInfo") << "Field name: " << field_name;
			//	}
			//}

			if (gst_structure_has_field(structure, "framerate"))
			{
				int numerator, denominator;

				// Extract the framerate as a fraction
				if (gst_structure_get_fraction(structure, "framerate", &numerator, &denominator))
				{
					outFrameInfo.framerate = static_cast<double>(numerator) / denominator;
				}
			}

			if (gst_structure_has_field(structure, "width"))
			{
				gst_structure_get_int(structure, "width", &outFrameInfo.width);
			}

			if (gst_structure_has_field(structure, "height"))
			{
				gst_structure_get_int(structure, "height", &outFrameInfo.height);
			}

			if (gst_structure_has_field(structure, "format"))
			{
				outFrameInfo.bufferFormat = gst_structure_get_string(structure, "format");
				outFrameInfo.modeName = gst_structure_get_name(structure);
			}
		}

		return outFrameInfo.isValid();
	}

	bool hasFrameInfoChanged(VideoModeConfigConstPtr videoMode, const GstVideoFrameInfo& frameInfo)
	{
		return !videoMode ||
			videoMode->bufferPixelWidth != frameInfo.width ||
			videoMode->bufferPixelHeight != frameInfo.height ||
			videoMode->frameRate != frameInfo.framerate ||
			videoMode->bufferFormat != frameInfo.bufferFormat;
	}

	VideoModeConfigPtr createVideoModeConfig(
		const GstVideoFrameInfo& frameInfo,
		GStreamerVideoConfigPtr cfg)
	{
		VideoModeConfigPtr outVideoMode = std::make_shared<VideoModeConfig>();

		outVideoMode->modeName= frameInfo.modeName;
		outVideoMode->frameRate= frameInfo.framerate;
		outVideoMode->bufferPixelWidth= frameInfo.width;
		outVideoMode->bufferPixelHeight= frameInfo.height;
		outVideoMode->bufferFormat= frameInfo.bufferFormat;
		outVideoMode->isBufferMirrored= false;
		outVideoMode->isFrameMirrored= false;
		outVideoMode->frameSections.push_back({0, 0});

		outVideoMode->intrinsics.setMonoIntrinsics(cfg->cameraIntrinsics);

		return outVideoMode;
	}
}

// -- GStreamer Video Device -----
GStreamerVideoDevice::GStreamerVideoDevice(
	const int cameraIndex,
	GStreamerVideoConfigPtr cfg,
	class IVideoSourceListener* listener)
	: m_cameraIndex(cameraIndex)
	, m_cfg(cfg)
	, m_videoSourceListener(listener)
	, m_impl(new GStreamerVideoDeviceImpl)
	, m_bIsStreaming(false)
{
}

GStreamerVideoDevice::~GStreamerVideoDevice()
{
	close();
	delete m_impl;
}

static std::string buildGStreamerPipelineString(GStreamerVideoConfigPtr cfg)
{
	std::stringstream ss;
	ss << cfg->getSourcePluginString() << " ";
	ss << "location=" << cfg->getFullURIPath() << " ";
	ss << "latency = 0 ";
	ss << "buffer-mode=auto ";
	ss << "!decodebin ";
	ss << "!videoconvert ";
	ss << "!video/x-raw,format=BGR ";
	ss << "!appsink name=sink";

	return ss.str();
}

bool GStreamerVideoDevice::open()
{
	if (getIsOpen())
	{
		return true;
	}

	GError* error = nullptr;
	std::string pipelineString= buildGStreamerPipelineString(m_cfg);
	m_impl->pipeline = gst_parse_launch(pipelineString.c_str(), &error);
	if (error)
	{
		MIKAN_LOG_INFO("GStreamerVideoDevice::open") << "Failed to create pipeline: " << error->message;
		g_error_free(error);
		close();
		return false;
	}

	m_impl->appsink = gst_bin_get_by_name(GST_BIN(m_impl->pipeline), "sink");
	if (!m_impl->appsink)
	{
		MIKAN_LOG_INFO("GStreamerVideoDevice::open") << "Failed to get appsink from pipeline!";
		close();
		return false;
	}

	// add a message handler
	m_impl->bus = gst_pipeline_get_bus(GST_PIPELINE(m_impl->pipeline));
	if (!m_impl->bus)
	{
		MIKAN_LOG_INFO("GStreamerVideoDevice::open") << "Failed to get bus from pipeline!";
		close();
		return false;
	}

	// Register the bus callback
	gst_bus_add_watch(m_impl->bus, GStreamerVideoDeviceImpl::busCallback, this);

	gst_app_sink_set_emit_signals(GST_APP_SINK(m_impl->appsink), FALSE);
	gst_app_sink_set_drop(GST_APP_SINK(m_impl->appsink), TRUE);
	gst_app_sink_set_max_buffers(GST_APP_SINK(m_impl->appsink), 1);

	return true;
}

bool GStreamerVideoDevice::getIsOpen() const
{
	return m_impl->pipeline != nullptr && m_impl->appsink != nullptr && m_impl->bus != nullptr;
}


void GStreamerVideoDevice::tryPullSample(
	VideoModeConfigPtr inVideoMode,
	VideoModeChangedCallback onVideoModeChanged)
{
	assert(getIsOpen());

	GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_impl->appsink), 0);
	if (sample)
	{
		GstBuffer* buffer = gst_sample_get_buffer(sample);
		GstCaps* caps = gst_sample_get_caps(sample);
		GstVideoFrameInfo frameInfo;
		if (GStreamerUtils::extractVideoFrameInfo(caps, frameInfo))
		{
			// See if the new frame info is different from the current video mode settings
			if (GStreamerUtils::hasFrameInfoChanged(inVideoMode, frameInfo))
			{
				// Let the caller know about the new video mode
				// so that it can apply it before the next frame is received
				VideoModeConfigPtr newVideoMode= 
					GStreamerUtils::createVideoModeConfig(frameInfo, m_cfg);
				onVideoModeChanged(newVideoMode);
			}

			GstMapInfo map;
			if (gst_buffer_map(buffer, &map, GST_MAP_READ))
			{
				IVideoSourceListener::FrameBuffer bufferInfo;
				bufferInfo.data= map.data;
				bufferInfo.byte_count = map.size;

				// Notify the listener that a new frame has been received
				m_videoSourceListener->notifyVideoFrameReceived(bufferInfo);

				// Create an OpenCV Mat from the buffer data
				gst_buffer_unmap(buffer, &map);
			}
			else
			{
				MIKAN_LOG_ERROR("GStreamerVideoDevice::tryPullSample") << "Failed to map buffer!";
			}
		}

		gst_sample_unref(sample);
	}
}

void GStreamerVideoDevice::close()
{
	// Make sure the pipeline is stopped first
	if (m_impl->pipeline != nullptr)
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
	}

	// Destroy the bus
	if (m_impl->bus != nullptr)
	{
		gst_object_unref(m_impl->bus);
		m_impl->bus= nullptr;
	}

	// Destroy the appsink
	if (m_impl->appsink != nullptr)
	{	
		gst_object_unref(m_impl->appsink);
		m_impl->appsink= nullptr;
	}

	// Destroy the pipeline
	if (m_impl->pipeline != nullptr)
	{
		gst_object_unref(m_impl->pipeline);
		m_impl->pipeline= nullptr;
	}
}

bool GStreamerVideoDevice::startVideoStream()
{
	if (getIsOpen())
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_PLAYING);
		m_bIsStreaming= true;
		return true;
	}

	return false;
}

bool GStreamerVideoDevice::getIsVideoStreaming()
{
	return m_bIsStreaming;
}

void GStreamerVideoDevice::stopVideoStream()
{
	if (getIsOpen())
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_PAUSED);
	}

	m_bIsStreaming= false;
}