#include "MikanGStreamerVideoDevice.h"
#include "MikanGStreamerVideoDeviceManager.h"

#include "Logger.h"
#include "MemoryUtils.h"

#include <gst/gst.h>
#include <gst/gstelement.h>
#include <gst/gstbus.h>
#include <gst/app/gstappsink.h>

#include "assert.h"

const std::string g_GStreamerProtocolStrings[(int)eNetworkVideoProtocol::COUNT] = {
	"rtmp",
	"rtsp"
};

// -- struct GStreamerImpl -----
struct GStreamerImpl
{
	eNetworkVideoProtocol protocol;
	std::string address;
	std::string path;
	int port;

	GstElement* pipeline;
	GstElement* appsink;
	GstBus* bus;

	GStreamerImpl(const NetworkVideoConnectionSettings& inSettings)
		: protocol(inSettings.protocol)
		, address(inSettings.address)
		, path(inSettings.path)
		, port(inSettings.port)
		, pipeline(nullptr)
		, appsink(nullptr)
		, bus(nullptr)
	{
	}

	std::string getSourcePluginString() const
	{
		if (protocol != eNetworkVideoProtocol::INVALID)
		{
			std::string protocolString = g_GStreamerProtocolStrings[(int)protocol];

			return protocolString + "src";
		}

		return "UNKNOWN";
	}

	std::string getURIProtocolString() const
	{
		if (protocol != eNetworkVideoProtocol::INVALID)
		{
			return g_GStreamerProtocolStrings[(int)protocol];
		}

		return "UNKNOWN";
	}

	std::string getFullURIPath() const
	{
		std::stringstream ss;

		ss << getURIProtocolString() << "://";
		ss << address << ":";
		ss << port << "/";
		ss << path;

		return ss.str();
	}

	std::string buildGStreamerPipelineString()
	{
		std::stringstream ss;
		ss << getSourcePluginString() << " ";
		ss << "location=\"" << getFullURIPath() << "\" ";
		ss << "latency=0 ";
		ss << "buffer-mode=auto ";
		ss << "! decodebin ";
		ss << "! videoconvert ";
		ss << "! video/x-raw,format=BGR ";
		ss << "! appsink name=sink";

		return ss.str();
	}

	static bool isFrameInfoValid(const NetworkVideoStreamProperties& frameInfo)
	{
		return
			frameInfo.width > 0 &&
			frameInfo.height > 0 &&
			frameInfo.frame_rate_numerator > 0 &&
			frameInfo.frame_rate_demonenator > 0 &&
			frameInfo.bufferFormat[0] != '\0' &&
			frameInfo.name[0] != '\0';
	}

	static bool hasFrameInfoChanged(
		const NetworkVideoStreamProperties& oldFrameInfo,
		const NetworkVideoStreamProperties& frameInfo)
	{
		return
			oldFrameInfo.width != frameInfo.width ||
			oldFrameInfo.height != frameInfo.height ||
			oldFrameInfo.frame_rate_numerator != frameInfo.frame_rate_numerator ||
			oldFrameInfo.frame_rate_demonenator != frameInfo.frame_rate_demonenator ||
			strncmp(oldFrameInfo.bufferFormat, frameInfo.bufferFormat, sizeof(oldFrameInfo.bufferFormat)) != 0 ||
			strncmp(oldFrameInfo.name, frameInfo.name, sizeof(oldFrameInfo.name)) != 0;
	}

	static bool extractVideoFrameInfo(GstCaps* caps, NetworkVideoStreamProperties& outFrameInfo)
	{
		std::memset(&outFrameInfo, 0, sizeof(NetworkVideoStreamProperties));

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
					outFrameInfo.frame_rate_numerator= numerator;
					outFrameInfo.frame_rate_demonenator = denominator;
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
				strncpy(
					outFrameInfo.bufferFormat,
					gst_structure_get_string(structure, "format"),
					sizeof(outFrameInfo.bufferFormat));
				strncpy(
					outFrameInfo.name,
					gst_structure_get_name(structure),
					sizeof(outFrameInfo.name));
			}
		}

		return isFrameInfoValid(outFrameInfo);
	}

	static gboolean busCallback(GstBus* bus, GstMessage* msg, gpointer data)
	{
		MikanGStreamerVideoDevice* videoDevice = reinterpret_cast<MikanGStreamerVideoDevice*>(data);

		switch (GST_MESSAGE_TYPE(msg))
		{
		case GST_MESSAGE_EOS:
		{
			MIKAN_LOG_INFO("bus_call") << "End of stream";
			videoDevice->close();
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

// -- MikanUsbVideoDevice -----
MikanGStreamerVideoDevice::MikanGStreamerVideoDevice(
	MikanGStreamerVideoDeviceManager* ownerDeviceManager,
	const NetworkVideoConnectionSettings& connectionInfo)
	: m_ownerDeviceManager(ownerDeviceManager)
	, m_connectionInfo(connectionInfo)
	, m_url(constructURL(connectionInfo))
	, m_impl(new GStreamerImpl(connectionInfo))
	, m_bIsStreaming(false)
{
	std::memset(&m_streamInfo, 0, sizeof(NetworkVideoStreamProperties));
}

MikanGStreamerVideoDevice::~MikanGStreamerVideoDevice()
{
	close();
	delete m_impl;
}

std::string MikanGStreamerVideoDevice::constructURL(
	const NetworkVideoConnectionSettings& connectionInfo)
{
	std::string protocolStr;
	switch (connectionInfo.protocol)
	{
	case eNetworkVideoProtocol::RTMP:
		protocolStr = "rtmp";
		break;
	case eNetworkVideoProtocol::RTSP:
		protocolStr = "rtsp";
		break;
	default:
		protocolStr = "unknown";
		break;
	}

	std::string url = protocolStr + "://";
	if (connectionInfo.address && connectionInfo.address[0] != '\0')
	{
		url += connectionInfo.address;
	}

	if (connectionInfo.port > 0)
	{
		url += ":" + std::to_string(connectionInfo.port);
	}

	if (connectionInfo.path && connectionInfo.path[0] != '\0')
	{
		if (connectionInfo.path[0] != '/')
		{
			url += "/";
		}

		url += connectionInfo.path;
	}

	return url;
}

// -- Device Listener
void MikanGStreamerVideoDevice::addListener(INetworkVideoDeviceListener* listener)
{
	m_listeners.insert(listener);
}

void MikanGStreamerVideoDevice::removeListener(INetworkVideoDeviceListener* listener)
{
	m_listeners.erase(listener);
}

// -- Device Properties
const char* MikanGStreamerVideoDevice::getDevicePath() const
{
	return m_url.c_str();
}

const char* MikanGStreamerVideoDevice::getFriendlyName() const
{
	return getDevicePath();
}

bool MikanGStreamerVideoDevice::getStreamProperties(NetworkVideoStreamProperties& outProperties) const
{
	if (getIsOpen() && GStreamerImpl::isFrameInfoValid(m_streamInfo))
	{
		outProperties = m_streamInfo;
		return true;
	}

	return false;
}

// -- Device Activation
bool MikanGStreamerVideoDevice::open()
{
	if (getIsOpen())
	{
		return true;
	}

	GError* error = nullptr;
	std::string pipelineString = m_impl->buildGStreamerPipelineString();
	m_impl->pipeline = gst_parse_launch(pipelineString.c_str(), &error);
	if (error)
	{
		MIKAN_LOG_INFO("MikanGStreamerVideoDevice::open") << "Failed to create pipeline: " << error->message;
		g_error_free(error);
		close();
		return false;
	}

	m_impl->appsink = gst_bin_get_by_name(GST_BIN(m_impl->pipeline), "sink");
	if (!m_impl->appsink)
	{
		MIKAN_LOG_INFO("MikanGStreamerVideoDevice::open") << "Failed to get appsink from pipeline!";
		close();
		return false;
	}

	// add a message handler
	m_impl->bus = gst_pipeline_get_bus(GST_PIPELINE(m_impl->pipeline));
	if (!m_impl->bus)
	{
		MIKAN_LOG_INFO("MikanGStreamerVideoDevice::open") << "Failed to get bus from pipeline!";
		close();
		return false;
	}

	// Register the bus callback
	gst_bus_add_watch(m_impl->bus, GStreamerImpl::busCallback, this);

	gst_app_sink_set_emit_signals(GST_APP_SINK(m_impl->appsink), FALSE);
	gst_app_sink_set_drop(GST_APP_SINK(m_impl->appsink), TRUE);
	gst_app_sink_set_max_buffers(GST_APP_SINK(m_impl->appsink), 1);

	return true;
}

bool MikanGStreamerVideoDevice::getIsOpen() const
{
	return m_impl->pipeline != nullptr && m_impl->appsink != nullptr && m_impl->bus != nullptr;
}

void MikanGStreamerVideoDevice::update(float deltaSeconds)
{
	assert(getIsOpen());

	GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_impl->appsink), 0);
	if (sample)
	{
		GstBuffer* buffer = gst_sample_get_buffer(sample);
		GstCaps* caps = gst_sample_get_caps(sample);
		NetworkVideoStreamProperties newFrameInfo;
		if (GStreamerImpl::extractVideoFrameInfo(caps, newFrameInfo))
		{
			// See if the new frame info is different from the current video mode settings
			if (GStreamerImpl::hasFrameInfoChanged(m_streamInfo, newFrameInfo))
			{
				// Let the listener know about the new video mode
				// so that it can apply it before the next frame is received
				m_streamInfo = newFrameInfo;
				notifyVideoModePropertiesChanged();
			}

			GstMapInfo map;
			if (gst_buffer_map(buffer, &map, GST_MAP_READ))
			{
				NetworkVideoFrameBuffer bufferInfo;
				bufferInfo.data = map.data;
				bufferInfo.byte_count = map.size;

				// Notify the listener that a new frame has been received
				notifyVideoFrameReceived(bufferInfo);

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

void MikanGStreamerVideoDevice::notifyVideoDeviceClosed()
{
	if (getIsOpen())
	{
		for (INetworkVideoDeviceListener* listener : m_listeners)
		{
			listener->notifyVideoDeviceClosed(this);
		}
	}
}

void MikanGStreamerVideoDevice::notifyVideoModePropertiesChanged()
{
	for (INetworkVideoDeviceListener* listener : m_listeners)
	{
		listener->notifyVideoModePropertiesChanged(this);
	}
}

void MikanGStreamerVideoDevice::notifyVideoFrameReceived(const NetworkVideoFrameBuffer& newBuffer)
{
	for (INetworkVideoDeviceListener* listener : m_listeners)
	{
		listener->notifyVideoFrameReceived(newBuffer);
	}
}

void MikanGStreamerVideoDevice::close()
{
	notifyVideoDeviceClosed();

	// Make sure the pipeline is stopped first
	if (m_impl->pipeline != nullptr)
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_NULL);
	}

	// Destroy the bus
	if (m_impl->bus != nullptr)
	{
		gst_object_unref(m_impl->bus);
		m_impl->bus = nullptr;
	}

	// Destroy the appsink
	if (m_impl->appsink != nullptr)
	{
		gst_object_unref(m_impl->appsink);
		m_impl->appsink = nullptr;
	}

	// Destroy the pipeline
	if (m_impl->pipeline != nullptr)
	{
		gst_object_unref(m_impl->pipeline);
		m_impl->pipeline = nullptr;
	}
}

// -- Video Settings
bool MikanGStreamerVideoDevice::isVideoSettingSupported(const eVideoSettingType property_type) const
{
	return false;
}

bool MikanGStreamerVideoDevice::getVideoSettingConstraint(
	const eVideoSettingType property_type, 
	VideoSettingConstraint& outConstraint) const
{
	return false;
}

void MikanGStreamerVideoDevice::setVideoSetting(const eVideoSettingType property_type, int desired_value)
{
	// Not supported
}

int MikanGStreamerVideoDevice::getVideoSetting(const eVideoSettingType property_type) const
{
	return -1;
}

// -- Video Streaming
eVideoStreamingStatus MikanGStreamerVideoDevice::startVideoStream()
{
	if (getIsOpen())
	{
		eVideoStreamingStatus status= getVideoStreamingStatus();

		if (status == eVideoStreamingStatus::stopped ||
			status == eVideoStreamingStatus::failed)
		{
			gst_element_set_state(m_impl->pipeline, GST_STATE_PLAYING);
			m_bIsStreaming = true;

			status = eVideoStreamingStatus::started;
		}

		return status;
	}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus MikanGStreamerVideoDevice::getVideoStreamingStatus() const
{
	if (getIsOpen())
	{
		return m_bIsStreaming
			? eVideoStreamingStatus::started
			: eVideoStreamingStatus::stopped;
	}

	return eVideoStreamingStatus::failed;
}

void MikanGStreamerVideoDevice::stopVideoStream()
{
	if (getIsOpen())
	{
		gst_element_set_state(m_impl->pipeline, GST_STATE_PAUSED);
	}

	m_bIsStreaming = false;
}
