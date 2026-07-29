#pragma once

// -- includes -----
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <vector>

#include "ARKitPoseReceiver.h" // ARKitPoseFrame

// A frame-correlated bundle of video + (optional) pose for the same frameSeq,
// produced once the bundle is complete or has gone stale (see ARKitFrameCorrelator
// below). hasVideo unset / pose empty represents a component that never arrived in
// time - not necessarily an error; downstream consumers decide what a partial
// bundle is still useful for (e.g. pose alone still drives camera tracking even
// without video - see IFrameCoupledPoseProvider, ticket E4).
struct ARKitFrameBundle
{
	uint32_t frameSeq= 0;
	uint64_t timestampUs= 0;

	bool hasVideo= false;
	// Opaque placeholder for the decoded video frame handle Track C (the GStreamer
	// pipeline / MikanARKitVideoDevice) will eventually own - this correlator does
	// not interpret or manage its lifetime, only tracks whether it arrived. Update
	// to a real typed handle once Track C exists.
	void* videoFrameHandle= nullptr;

	std::optional<ARKitPoseFrame> pose;
};

// Fans in video/pose arrival notifications - from two independent producer threads
// (Track C's GStreamer pipeline thread, ARKitPoseReceiver's worker thread) - keyed
// by the shared frameSeq, firing a bundle callback once a frame is complete or has
// gone stale.
//
// Purely reactive: unlike ARKitPoseReceiver, this class owns no socket and no
// worker thread of its own. sweepStaleFrames() must be driven periodically by
// whatever owns this correlator (e.g. a device's per-tick update(deltaSeconds),
// matching the existing INetworkVideoDevice::update pattern used elsewhere in this
// codebase).
//
// Thread-safe: all public methods may be called from any thread, concurrently.
class ARKitFrameCorrelator
{
public:
	// staleTimeout comfortably exceeds normal video decode latency and network
	// jitter, so a pose packet that's simply running a little behind the video
	// packet for the same frameSeq isn't routinely delivered as a video-only
	// partial bundle under normal conditions.
	explicit ARKitFrameCorrelator(std::chrono::milliseconds staleTimeout= std::chrono::milliseconds(1000));

	void setBundleCallback(std::function<void(ARKitFrameBundle)> callback);

	void notifyVideoArrived(uint32_t frameSeq, uint64_t timestampUs, void* videoFrameHandle);
	void notifyPoseArrived(ARKitPoseFrame pose);

	// Delivers (as partial bundles) and evicts any pending frameSeq entries older
	// than staleTimeout as of `now`. `now` is passed explicitly (not read
	// internally) so tests can simulate time passing deterministically.
	void sweepStaleFrames(std::chrono::steady_clock::time_point now);

	size_t getPendingFrameCount() const;
	size_t getCompletedFrameCount() const;
	size_t getPartialDeliveredFrameCount() const;

private:
	struct PendingEntry
	{
		bool hasVideo= false;
		void* videoFrameHandle= nullptr;
		uint64_t videoTimestampUs= 0;

		std::optional<ARKitPoseFrame> pose;

		std::chrono::steady_clock::time_point firstArrival;
	};

	// The following helpers all require m_mutex to already be held by the caller.
	std::map<uint32_t, PendingEntry>::iterator getOrCreateLocked(uint32_t frameSeq);
	bool isComplete(const PendingEntry& entry) const;
	ARKitFrameBundle buildBundle(uint32_t frameSeq, const PendingEntry& entry) const;
	// Erases `it` and returns its bundle if complete, else leaves it pending and
	// returns nullopt. Never invokes the callback itself - callers must unlock
	// m_mutex first, both to avoid blocking other producer threads' notify*() calls
	// for the callback's duration and to avoid deadlocking if the callback
	// re-enters this correlator.
	std::optional<ARKitFrameBundle> tryFinalizeLocked(std::map<uint32_t, PendingEntry>::iterator it);

	void deliverIfPresent(std::optional<ARKitFrameBundle>& bundle);

	mutable std::mutex m_mutex;
	std::map<uint32_t, PendingEntry> m_pending;
	std::chrono::milliseconds m_staleTimeout;
	std::function<void(ARKitFrameBundle)> m_callback;
	size_t m_completedCount= 0;
	size_t m_partialDeliveredCount= 0;
};
