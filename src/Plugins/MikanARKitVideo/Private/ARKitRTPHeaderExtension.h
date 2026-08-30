#pragma once

#include <gst/gst.h>
#include <gst/rtp/gstrtphdrext.h>

#include <cstdint>

// GstRTPHeaderExtension subclass that reads the frameSeq/captureTimestampUs (and,
// optionally, frame-coupled pose) payload carried by every RTP packet of the video
// channel (basePort+0, extension id 1 - see the Wire Protocol Reference) and
// attaches it to the depayloaded output buffer as an ARKitFrameSeqMeta. Tolerant
// of two payload shapes, dispatched on size: the legacy 12-byte frameSeq-only
// ARKitRTPExtensionPayload (RFC 5285 one-byte header) and the newer 100-byte
// pose-bearing ARKitPoseInRTPPayload (RFC 5285 two-byte header, since 100 bytes
// exceeds the one-byte header's 16-byte payload limit) - both in
// ARKitWireProtocol.h. Mikan only ever receives this stream, so only the read
// direction is implemented; write() is a stub.
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
// Custom GstMeta carrying the frameSeq/captureTimestampUs (and, optionally,
// frame-coupled pose) extracted from the RTP header extension onto the
// depayloaded buffer. GstReferenceTimestampMeta can't carry an explicit frameSeq
// (only a single GstClockTime + classifying GstCaps), so a small dedicated meta
// type is used instead.
typedef struct _ARKitFrameSeqMeta
{
	GstMeta meta;
	uint32_t frameSeq;
	uint64_t captureTimestampUs;

	// Set only when this frame's RTP extension carried the newer 100-byte
	// pose-bearing payload (ARKitPoseInRTPPayload) rather than the legacy 12-byte
	// frameSeq-only one - false leaves the fields below unpopulated/zeroed.
	bool hasPose;
	float transform[16]; // row-major 4x4, camera-to-world
	float fx;
	float fy;
	float cx;
	float cy;
	float imageWidth;
	float imageHeight;
} ARKitFrameSeqMeta;

GType arkit_frame_seq_meta_api_get_type(void);
#define ARKIT_FRAME_SEQ_META_API_TYPE (arkit_frame_seq_meta_api_get_type())

const GstMetaInfo* arkit_frame_seq_meta_get_info(void);
#define ARKIT_FRAME_SEQ_META_INFO (arkit_frame_seq_meta_get_info())

ARKitFrameSeqMeta* arkit_buffer_add_frame_seq_meta(GstBuffer* buffer, uint32_t frameSeq, uint64_t captureTimestampUs);

// Same as above, but also attaches frame-coupled pose data (from a 100-byte
// ARKitPoseInRTPPayload RTP extension). transform must point to 16 row-major floats.
ARKitFrameSeqMeta* arkit_buffer_add_frame_seq_meta_with_pose(GstBuffer* buffer, uint32_t frameSeq,
															 uint64_t captureTimestampUs, const float transform[16],
															 float fx, float fy, float cx, float cy, float imageWidth,
															 float imageHeight);

// Returns nullptr if the buffer has no such meta (e.g. the extension never arrived
// on this frame, or it was stripped by an intervening element).
ARKitFrameSeqMeta* arkit_buffer_get_frame_seq_meta(GstBuffer* buffer);
