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

	IVideoSourceListener* videoSourceListener= nullptr;
	GstElement* pipeline= nullptr;
	GstElement* appsink= nullptr;

	GStreamerVideoDeviceImpl() = default;
};

// -- GStreamer Video Device -----
GStreamerVideoDevice::GStreamerVideoDevice(const int deviceIndex, const std::string& cameraURI)
	: m_deviceIndex(deviceIndex)
	, m_cameraURI(cameraURI)
	, m_impl(new GStreamerVideoDeviceImpl)
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
	ss << "! decodebin ";
	ss << "! videoconvert ";
	ss << "! appsink name=sink";
	//ss << "!rtph264depay ";
	//ss << "!h264parse ";
	//ss << "!d3d11h264dec ";
	//ss << "!video.";

	return ss.str();
}

bool GStreamerVideoDevice::open(
	GStreamerVideoConfigPtr cfg,
	IVideoSourceListener* videoSourceListener)
{
	if (getIsOpen())
	{
		return true;
	}

	// Close the device if it's currently open
	if (getIsOpen())
	{
		close();
	}

	GError* error = nullptr;
	std::string pipelineString= buildGStreamerPipelineString(cfg);
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

	gst_app_sink_set_emit_signals(GST_APP_SINK(m_impl->appsink), FALSE);
	gst_app_sink_set_drop(GST_APP_SINK(m_impl->appsink), TRUE);
	gst_app_sink_set_max_buffers(GST_APP_SINK(m_impl->appsink), 1);

	// Remember the video source listener to post frames back to 
	m_impl->videoSourceListener = videoSourceListener;

	return true;
}

bool GStreamerVideoDevice::getIsOpen() const
{
	return m_impl->pipeline != nullptr && m_impl->appsink != nullptr;
}

void GStreamerVideoDevice::tryPullSample()
{
	assert(getIsOpen());

	GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_impl->appsink), 0);
	if (sample)
	{
		GstBuffer* buffer = gst_sample_get_buffer(sample);
		GstCaps* caps = gst_sample_get_caps(sample);
		if (caps)
		{
			GstStructure* structure = gst_caps_get_structure(caps, 0);
			int width, height;
			gst_structure_get_int(structure, "width", &width);
			gst_structure_get_int(structure, "height", &height);
			//TODO: Bit Depth
			//TODO: Frame Rate

			GstMapInfo map;
			if (gst_buffer_map(buffer, &map, GST_MAP_READ))
			{
				// Notify the listener that a new frame has been received
				m_impl->videoSourceListener->notifyVideoFrameReceived(map.data);

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
	if (m_impl->pipeline != nullptr)
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
	}

	if (m_impl->appsink != nullptr)
	{	
		gst_object_unref(m_impl->appsink);
		m_impl->appsink= nullptr;
	}

	if (m_impl->pipeline != nullptr)
	{
		gst_object_unref(m_impl->pipeline);
		m_impl->pipeline= nullptr;
	}
}

bool GStreamerVideoDevice::startVideoStream()
{
	if (getIsOpen() && !getIsVideoStreaming())
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_PLAYING);
	}

	return false;
}

bool GStreamerVideoDevice::getIsVideoStreaming()
{
	if (getIsOpen())
	{
		GstState state;
		GstStateChangeReturn ret = gst_element_get_state(m_impl->pipeline, &state, nullptr, 0);

		if (ret == GST_STATE_CHANGE_SUCCESS)
		{
			return state == GST_STATE_PLAYING;
		}
	}

	return false;
}

void GStreamerVideoDevice::stopVideoStream()
{
	if (getIsOpen())
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
	}
}