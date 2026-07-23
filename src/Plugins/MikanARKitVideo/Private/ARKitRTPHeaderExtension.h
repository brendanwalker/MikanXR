#pragma once

#include <gst/gst.h>
#include <gst/rtp/gstrtphdrext.h>

#include <cstdint>

// GstRTPHeaderExtension subclass that reads the 12-byte frameSeq/captureTimestampUs
// payload (ARKitWireProtocol.h's ARKitRTPExtensionPayload) carried by every RTP
// packet of the video channel (basePort+0, RFC 5285 one-byte header, extension id 1
// - see the Wire Protocol Reference) and attaches it to the depayloaded output
// buffer as an ARKitFrameSeqMeta. Mikan only ever receives this stream, so only the
// read direction is implemented; write() is a stub.
//
// G_DECLARE_FINAL_TYPE below generates arkit_rtp_header_extension_get_type(), the
// ARKitRTPHeaderExtension/ARKitRTPHeaderExtensionClass typedefs, and the
// ARKIT_RTP_HEADER_EXTENSION(obj)/ARKIT_IS_RTP_HEADER_EXTENSION(obj) cast/check
// macros - but NOT a _TYPE_ macro, so that's defined explicitly below.
G_DECLARE_FINAL_TYPE(ARKitRTPHeaderExtension, arkit_rtp_header_extension, ARKIT, RTP_HEADER_EXTENSION,
					 GstRTPHeaderExtension)
#define ARKIT_TYPE_RTP_HEADER_EXTENSION (arkit_rtp_header_extension_get_type())

// Not IANA-registered (this protocol is private to MikanXR/MikanARStreamer), but
// RFC 5285 only requires the URI be unique within the session's SDP negotiation,
// which is out of scope here since the extension id is fixed and hardcoded on both
// ends rather than negotiated.
#define ARKIT_RTP_HDREXT_URI "urn:mikanxr:rtp-hdrext:arkit-frameseq"

// The fixed RFC 5285 one-byte extension id both sides use for this extension - see
// the Wire Protocol Reference in the implementation plan.
constexpr guint kARKitRTPHeaderExtensionId= 1;

GstRTPHeaderExtension* arkit_rtp_header_extension_new(void);

// -- ARKitFrameSeqMeta -----
// Custom GstMeta carrying the frameSeq/captureTimestampUs extracted from the RTP
// header extension onto the depayloaded buffer. GstReferenceTimestampMeta can't
// carry an explicit frameSeq (only a single GstClockTime + classifying GstCaps), so
// a small dedicated meta type is used instead.
typedef struct _ARKitFrameSeqMeta
{
	GstMeta meta;
	uint32_t frameSeq;
	uint64_t captureTimestampUs;
} ARKitFrameSeqMeta;

GType arkit_frame_seq_meta_api_get_type(void);
#define ARKIT_FRAME_SEQ_META_API_TYPE (arkit_frame_seq_meta_api_get_type())

const GstMetaInfo* arkit_frame_seq_meta_get_info(void);
#define ARKIT_FRAME_SEQ_META_INFO (arkit_frame_seq_meta_get_info())

ARKitFrameSeqMeta* arkit_buffer_add_frame_seq_meta(GstBuffer* buffer, uint32_t frameSeq, uint64_t captureTimestampUs);

// Returns nullptr if the buffer has no such meta (e.g. the extension never arrived
// on this frame, or it was stripped by an intervening element).
ARKitFrameSeqMeta* arkit_buffer_get_frame_seq_meta(GstBuffer* buffer);
