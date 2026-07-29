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
	self->hasPose= false;
	for (int i= 0; i < 16; ++i)
		self->transform[i]= 0.f;
	self->fx= self->fy= self->cx= self->cy= 0.f;
	self->imageWidth= self->imageHeight= 0.f;
	return TRUE;
}

// Called by gst_buffer_copy_into()/subbuffer/etc. when GST_BUFFER_COPY_META is
// requested - this is the mechanism that lets the meta survive across the
// depay -> parse -> decoder chain's internal buffer transforms, the same way
// GstReferenceTimestampMeta (e.g. the ONVIF timestamp extension) propagates.
//
// Must copy every field, not just frameSeq/captureTimestampUs - this is the
// single highest-risk line in the pose-in-RTP reader: if hasPose/the pose fields
// aren't propagated here, pose silently vanishes across the
// rtph264depay -> h264parse -> nvh264dec buffer-copy chain with no error, just a
// permanently-false hasPose on the other side.
static gboolean arkit_frame_seq_meta_transform(GstBuffer* transbuf, GstMeta* meta, GstBuffer* /*buffer*/,
											   GQuark /*type*/, gpointer /*data*/)
{
	const ARKitFrameSeqMeta* srcMeta= reinterpret_cast<const ARKitFrameSeqMeta*>(meta);
	if (srcMeta->hasPose)
	{
		arkit_buffer_add_frame_seq_meta_with_pose(transbuf, srcMeta->frameSeq, srcMeta->captureTimestampUs,
												  srcMeta->transform, srcMeta->fx, srcMeta->fy, srcMeta->cx,
												  srcMeta->cy, srcMeta->imageWidth, srcMeta->imageHeight);
	}
	else
	{
		arkit_buffer_add_frame_seq_meta(transbuf, srcMeta->frameSeq, srcMeta->captureTimestampUs);
	}
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
		meta->hasPose= false;
	}

	return meta;
}

ARKitFrameSeqMeta* arkit_buffer_add_frame_seq_meta_with_pose(GstBuffer* buffer, uint32_t frameSeq,
															 uint64_t captureTimestampUs, const float transform[16],
															 float fx, float fy, float cx, float cy, float imageWidth,
															 float imageHeight)
{
	ARKitFrameSeqMeta* meta= arkit_buffer_add_frame_seq_meta(buffer, frameSeq, captureTimestampUs);

	if (meta != nullptr)
	{
		meta->hasPose= true;
		for (int i= 0; i < 16; ++i)
			meta->transform[i]= transform[i];
		meta->fx= fx;
		meta->fy= fy;
		meta->cx= cx;
		meta->cy= cy;
		meta->imageWidth= imageWidth;
		meta->imageHeight= imageHeight;
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
	// Both forms are supported (this is a flags enum, not exclusive): the legacy
	// 12-byte ARKitRTPExtensionPayload fits the one-byte header's 1-16 byte
	// payload range, but the newer 100-byte ARKitPoseInRTPPayload needs the
	// two-byte header's larger range instead.
	return static_cast<GstRTPHeaderExtensionFlags>(GST_RTP_HEADER_EXTENSION_ONE_BYTE
												   | GST_RTP_HEADER_EXTENSION_TWO_BYTE);
}

static gsize arkit_rtp_header_extension_get_max_size(GstRTPHeaderExtension* /*ext*/, const GstBuffer* /*input_meta*/)
{
	// The larger of the two payload shapes this reader accepts.
	return ARKitPoseInRTPPayload::kWireSize;
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
	// Dispatch on payload size: legacy 12-byte frameSeq-only, or the newer
	// 100-byte pose-bearing shape. Anything else is rejected rather than guessed.
	if (size == ARKitRTPExtensionPayload::kWireSize)
	{
		ARKitRTPExtensionPayload payload;
		if (!readARKitRTPExtensionPayload(data, size, payload))
			return FALSE;

		return arkit_buffer_add_frame_seq_meta(buffer, payload.frameSeq, payload.captureTimestampUs) != nullptr;
	}

	if (size == ARKitPoseInRTPPayload::kWireSize)
	{
		ARKitPoseInRTPPayload payload;
		if (!readARKitPoseInRTPPayload(data, size, payload))
			return FALSE;

		return arkit_buffer_add_frame_seq_meta_with_pose(buffer, payload.frameSeq, payload.captureTimestampUs,
														 payload.transform, payload.fx, payload.fy, payload.cx,
														 payload.cy, payload.imageWidth, payload.imageHeight)
			   != nullptr;
	}

	return FALSE;
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

	gst_element_class_set_static_metadata(elementClass, "ARKit frameSeq RTP Header Extension",
										  GST_RTP_HDREXT_ELEMENT_CLASS,
										  "Extracts the frameSeq/captureTimestampUs (and optionally pose) RTP header "
										  "extension carried by the MikanARStreamer iPhone app",
										  "MikanXR");
}

static void arkit_rtp_header_extension_init(ARKitRTPHeaderExtension* /*self*/) {}

GstRTPHeaderExtension* arkit_rtp_header_extension_new(void)
{
	return GST_RTP_HEADER_EXTENSION(g_object_new(ARKIT_TYPE_RTP_HEADER_EXTENSION, nullptr));
}
