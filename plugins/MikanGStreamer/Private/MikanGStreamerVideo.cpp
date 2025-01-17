// -- includes -----
#include "MikanGStreamerVideo.h"
#include "Logger.h"

#include <gst/gst.h>
#include <gst/gstelement.h>
#include <gst/gstbus.h>
#include <gst/app/gstappsink.h>

#include <sstream>
#include <string>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
#endif

// -- GStreamer Video Device -----
class MikanGStreamerVideoDevice : public IMikanGStreamerVideoDevice
{
public:
	MikanGStreamerVideoDevice(const MikanGStreamerSettings& settings)
		: m_settings(settings)
		, m_bIsStreaming(false)
	{

	}

	virtual ~MikanGStreamerVideoDevice()
	{
		close();
	}

	// Create / Destroy a GStreamer Video Pipeline based on the settings
	bool open() override
	{
		if (getIsOpen())
		{
			return true;
		}

		GError* error = nullptr;
		std::string pipelineString = buildGStreamerPipelineString();
		m_pipeline = gst_parse_launch(pipelineString.c_str(), &error);
		if (error)
		{
			MIKAN_LOG_INFO("GStreamerVideoDevice::open") << "Failed to create pipeline: " << error->message;
			g_error_free(error);
			close();
			return false;
		}

		m_appsink = gst_bin_get_by_name(GST_BIN(m_pipeline), "sink");
		if (!m_appsink)
		{
			MIKAN_LOG_INFO("GStreamerVideoDevice::open") << "Failed to get appsink from pipeline!";
			close();
			return false;
		}

		// add a message handler
		m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
		if (!m_bus)
		{
			MIKAN_LOG_INFO("GStreamerVideoDevice::open") << "Failed to get bus from pipeline!";
			close();
			return false;
		}

		// Register the bus callback
		gst_bus_add_watch(m_bus, MikanGStreamerVideoDevice::busCallback, this);

		gst_app_sink_set_emit_signals(GST_APP_SINK(m_appsink), FALSE);
		gst_app_sink_set_drop(GST_APP_SINK(m_appsink), TRUE);
		gst_app_sink_set_max_buffers(GST_APP_SINK(m_appsink), 1);

		return true;
	}

	bool getIsOpen() const override
	{
		return m_pipeline != nullptr && m_appsink != nullptr && m_bus != nullptr;
	}

	void close() override
	{
		// Make sure the pipeline is stopped first
		if (m_pipeline != nullptr)
		{
			gst_element_set_state(m_pipeline, GST_STATE_NULL);
		}

		// Destroy the bus
		if (m_bus != nullptr)
		{
			gst_object_unref(m_bus);
			m_bus = nullptr;
		}

		// Destroy the appsink
		if (m_appsink != nullptr)
		{
			gst_object_unref(m_appsink);
			m_appsink = nullptr;
		}

		// Destroy the pipeline
		if (m_pipeline != nullptr)
		{
			gst_object_unref(m_pipeline);
			m_pipeline = nullptr;
		}
	}

	// Start / Stop an opened pipeline
	bool startVideoStream() override
	{
		if (getIsOpen())
		{
			gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
			m_bIsStreaming = true;
			return true;
		}

		return false;
	}

	bool getIsVideoStreaming() override
	{
		return m_bIsStreaming;
	}

	void stopVideoStream() override
	{
		if (getIsOpen())
		{
			gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
		}

		m_bIsStreaming = false;
	}

	// Try and fetch the next video frame from a started pipeline
	void tryPullSample(
		const MikanGStreamerVideoMode& inVideoMode,
		void (*onVideoModeChanged)(const MikanGStreamerVideoMode& newVideoMode, void* userdata),
		void (*onVideoFrameReceived)(const MikanGStreamerBuffer& newBuffer, void* userdata),
		void* userdata) override
	{
		assert(getIsOpen());

		GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_appsink), 0);
		if (sample)
		{
			GstBuffer* buffer = gst_sample_get_buffer(sample);
			GstCaps* caps = gst_sample_get_caps(sample);
			MikanGStreamerVideoMode newFrameInfo;
			if (extractVideoFrameInfo(caps, newFrameInfo))
			{
				// See if the new frame info is different from the current video mode settings
				if (hasFrameInfoChanged(inVideoMode, newFrameInfo))
				{
					// Let the caller know about the new video mode
					// so that it can apply it before the next frame is received
					onVideoModeChanged(newFrameInfo, userdata);
				}

				GstMapInfo map;
				if (gst_buffer_map(buffer, &map, GST_MAP_READ))
				{
					MikanGStreamerBuffer bufferInfo;
					bufferInfo.data = map.data;
					bufferInfo.byte_count = map.size;

					// Notify the listener that a new frame has been received
					onVideoFrameReceived(bufferInfo, userdata);

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

protected:
	static gboolean busCallback(GstBus* bus, GstMessage* msg, gpointer data)
	{
		MikanGStreamerVideoDevice* videoDevice = reinterpret_cast<MikanGStreamerVideoDevice*>(data);

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

	std::string getSourcePluginString() const
	{
		if (m_settings.protocol != eGStreamerProtocol::INVALID)
		{
			std::string protocolString= k_szGStreamerProtocolStrings[(int)m_settings.protocol];

			return protocolString + "src";
		}

		return "UNKNOWN";
	}

	std::string getURIProtocolString() const
	{
		if (m_settings.protocol != eGStreamerProtocol::INVALID)
		{
			return k_szGStreamerProtocolStrings[(int)m_settings.protocol];
		}

		return "UNKNOWN";
	}

	std::string getFullURIPath() const
	{
		std::stringstream ss;

		ss << getURIProtocolString() << "://";
		ss << m_settings.address << ":";
		ss << m_settings.port << "/";
		ss << m_settings.path;

		return ss.str();
	}

	std::string buildGStreamerPipelineString()
	{
		std::stringstream ss;
		ss << getSourcePluginString() << " ";
		ss << "location=" << getFullURIPath() << " ";
		ss << "latency = 0 ";
		ss << "buffer-mode=auto ";
		ss << "!decodebin ";
		ss << "!videoconvert ";
		ss << "!video/x-raw,format=BGR ";
		ss << "!appsink name=sink";

		return ss.str();
	}

	static bool isFrameInfoValid(const MikanGStreamerVideoMode& frameInfo)
	{
		return
			frameInfo.bufferPixelWidth > 0 &&
			frameInfo.bufferPixelHeight > 0 &&
			frameInfo.frameRate > 0 &&
			frameInfo.bufferFormat[0] != '\0' &&
			frameInfo.modeName[0] != '\0';
	}

	static bool hasFrameInfoChanged(
		const MikanGStreamerVideoMode& oldFrameInfo,
		const MikanGStreamerVideoMode& frameInfo)
	{
		return
			oldFrameInfo.bufferPixelWidth != frameInfo.bufferPixelWidth ||
			oldFrameInfo.bufferPixelHeight != frameInfo.bufferPixelHeight ||
			oldFrameInfo.frameRate != frameInfo.frameRate ||
			strncmp(oldFrameInfo.bufferFormat, frameInfo.bufferFormat, sizeof(oldFrameInfo.bufferFormat)) != 0;
	}

	bool extractVideoFrameInfo(GstCaps* caps, MikanGStreamerVideoMode& outFrameInfo)
	{
		std::memset(&outFrameInfo, 0, sizeof(MikanGStreamerVideoMode));

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
					outFrameInfo.frameRate = static_cast<double>(numerator) / denominator;
				}
			}

			if (gst_structure_has_field(structure, "width"))
			{
				gst_structure_get_int(structure, "width", &outFrameInfo.bufferPixelWidth);
			}

			if (gst_structure_has_field(structure, "height"))
			{
				gst_structure_get_int(structure, "height", &outFrameInfo.bufferPixelHeight);
			}

			if (gst_structure_has_field(structure, "format"))
			{
				strncpy(
					outFrameInfo.bufferFormat, 
					gst_structure_get_string(structure, "format"), 
					sizeof(outFrameInfo.bufferFormat));
				strncpy(
					outFrameInfo.modeName, 
					gst_structure_get_name(structure), 
					sizeof(outFrameInfo.modeName));
			}
		}

		return isFrameInfoValid(outFrameInfo);
	}

private:
	MikanGStreamerSettings m_settings;
	bool m_bIsStreaming;

	GstElement* m_pipeline = nullptr;
	GstElement* m_appsink = nullptr;
	GstBus* m_bus = nullptr;
};

// C-API
IMikanGStreamerVideoDevice* MikanGStreamerVideoDeviceAllocate(const MikanGStreamerSettings& settings)
{
	return new MikanGStreamerVideoDevice(settings);
}

void MikanGStreamerVideoDeviceDispose(IMikanGStreamerVideoDevice* device)
{
	if (device != nullptr)
	{
		delete device;
	}
}