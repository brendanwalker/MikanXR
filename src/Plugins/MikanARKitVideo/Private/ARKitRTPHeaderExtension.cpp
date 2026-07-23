#include "ARKitRTPHeaderExtension.h"
#include "ARKitWireProtocol.h"

// -- ARKitFrameSeqMeta -----
GType arkit_frame_seq_meta_api_get_type(void)
{
	static GType type= 0;
	static const gchar* tags[]= {nullptr};

	if (g_once_init_enter(&type))
	{
		const GType newType= gst_meta_api_type_register("ARKitFrameSeqMetaAPI", tags);
		g_once_init_leave(&type, newType);
	}

	return type;
}

static gboolean arkit_frame_seq_meta_init(GstMeta* meta, gpointer /*params*/, GstBuffer* /*buffer*/)
{
	ARKitFrameSeqMeta* self= reinterpret_cast<ARKitFrameSeqMeta*>(meta);
	self->frameSeq= 0;
	self->captureTimestampUs= 0;
	return TRUE;
}

// Called by gst_buffer_copy_into()/subbuffer/etc. when GST_BUFFER_COPY_META is
// requested - this is the mechanism that lets the meta survive across the
// depay -> parse -> decoder chain's internal buffer transforms, the same way
// GstReferenceTimestampMeta (e.g. the ONVIF timestamp extension) propagates.
static gboolean arkit_frame_seq_meta_transform(GstBuffer* transbuf, GstMeta* meta, GstBuffer* /*buffer*/,
											   GQuark /*type*/, gpointer /*data*/)
{
	const ARKitFrameSeqMeta* srcMeta= reinterpret_cast<const ARKitFrameSeqMeta*>(meta);
	arkit_buffer_add_frame_seq_meta(transbuf, srcMeta->frameSeq, srcMeta->captureTimestampUs);
	return TRUE;
}

const GstMetaInfo* arkit_frame_seq_meta_get_info(void)
{
	static const GstMetaInfo* metaInfo= nullptr;

	if (g_once_init_enter(&metaInfo))
	{
		const GstMetaInfo* newInfo= gst_meta_register(ARKIT_FRAME_SEQ_META_API_TYPE, "ARKitFrameSeqMeta",
													  sizeof(ARKitFrameSeqMeta), arkit_frame_seq_meta_init,
													  /*free_func=*/nullptr, // POD data, nothing to release
													  arkit_frame_seq_meta_transform);
		g_once_init_leave(&metaInfo, newInfo);
	}

	return metaInfo;
}

ARKitFrameSeqMeta* arkit_buffer_add_frame_seq_meta(GstBuffer* buffer, uint32_t frameSeq, uint64_t captureTimestampUs)
{
	ARKitFrameSeqMeta* meta=
		reinterpret_cast<ARKitFrameSeqMeta*>(gst_buffer_add_meta(buffer, ARKIT_FRAME_SEQ_META_INFO, nullptr));

	if (meta != nullptr)
	{
		meta->frameSeq= frameSeq;
		meta->captureTimestampUs= captureTimestampUs;
	}

	return meta;
}

ARKitFrameSeqMeta* arkit_buffer_get_frame_seq_meta(GstBuffer* buffer)
{
	return reinterpret_cast<ARKitFrameSeqMeta*>(gst_buffer_get_meta(buffer, ARKIT_FRAME_SEQ_META_API_TYPE));
}

// -- ARKitRTPHeaderExtension -----
struct _ARKitRTPHeaderExtension
{
	GstRTPHeaderExtension parent_instance;
};

G_DEFINE_TYPE(ARKitRTPHeaderExtension, arkit_rtp_header_extension, GST_TYPE_RTP_HEADER_EXTENSION)

static GstRTPHeaderExtensionFlags arkit_rtp_header_extension_get_supported_flags(GstRTPHeaderExtension* /*ext*/)
{
	// ARKitRTPExtensionPayload::kWireSize (12 bytes) fits within the one-byte
	// header's 1-16 byte payload range.
	return GST_RTP_HEADER_EXTENSION_ONE_BYTE;
}

static gsize arkit_rtp_header_extension_get_max_size(GstRTPHeaderExtension* /*ext*/, const GstBuffer* /*input_meta*/)
{
	return ARKitRTPExtensionPayload::kWireSize;
}

static gssize arkit_rtp_header_extension_write(GstRTPHeaderExtension* /*ext*/, const GstBuffer* /*input_meta*/,
											   GstRTPHeaderExtensionFlags /*write_flags*/, GstBuffer* /*output*/,
											   guint8* /*data*/, gsize /*size*/)
{
	// Mikan is a receive-only consumer of this extension (the iPhone app is the
	// only sender - see Track A8) - writing was never needed for this ticket.
	return -1;
}

static gboolean arkit_rtp_header_extension_read(GstRTPHeaderExtension* /*ext*/,
												GstRTPHeaderExtensionFlags /*read_flags*/, const guint8* data,
												gsize size, GstBuffer* buffer)
{
	ARKitRTPExtensionPayload payload;
	if (!readARKitRTPExtensionPayload(data, size, payload))
		return FALSE;

	return arkit_buffer_add_frame_seq_meta(buffer, payload.frameSeq, payload.captureTimestampUs) != nullptr;
}

static void arkit_rtp_header_extension_class_init(ARKitRTPHeaderExtensionClass* klass)
{
	GstRTPHeaderExtensionClass* rtpExtClass= GST_RTP_HEADER_EXTENSION_CLASS(klass);
	GstElementClass* elementClass= GST_ELEMENT_CLASS(klass);

	rtpExtClass->get_supported_flags= arkit_rtp_header_extension_get_supported_flags;
	rtpExtClass->get_max_size= arkit_rtp_header_extension_get_max_size;
	rtpExtClass->write= arkit_rtp_header_extension_write;
	rtpExtClass->read= arkit_rtp_header_extension_read;

	gst_rtp_header_extension_class_set_uri(rtpExtClass, ARKIT_RTP_HDREXT_URI);

	gst_element_class_set_static_metadata(
		elementClass, "ARKit frameSeq RTP Header Extension", GST_RTP_HDREXT_ELEMENT_CLASS,
		"Extracts the frameSeq/captureTimestampUs RTP header extension carried by the MikanARStreamer iPhone app",
		"MikanXR");
}

static void arkit_rtp_header_extension_init(ARKitRTPHeaderExtension* /*self*/) {}

GstRTPHeaderExtension* arkit_rtp_header_extension_new(void)
{
	return GST_RTP_HEADER_EXTENSION(g_object_new(ARKIT_TYPE_RTP_HEADER_EXTENSION, nullptr));
}
