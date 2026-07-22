#include "ARKitFrameCorrelator.h"
#include "Logger.h"

ARKitFrameCorrelator::ARKitFrameCorrelator(bool depthStreamingEnabled, std::chrono::milliseconds staleTimeout)
	: m_depthStreamingEnabled(depthStreamingEnabled)
	, m_staleTimeout(staleTimeout)
{
}

void ARKitFrameCorrelator::setDepthStreamingEnabled(bool enabled) { m_depthStreamingEnabled.store(enabled); }

void ARKitFrameCorrelator::setBundleCallback(std::function<void(ARKitFrameBundle)> callback)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_callback= std::move(callback);
}

std::map<uint32_t, ARKitFrameCorrelator::PendingEntry>::iterator ARKitFrameCorrelator::getOrCreateLocked(
	uint32_t frameSeq)
{
	auto it= m_pending.find(frameSeq);
	if (it == m_pending.end())
	{
		PendingEntry entry;
		entry.firstArrival= std::chrono::steady_clock::now();
		it= m_pending.emplace(frameSeq, std::move(entry)).first;
	}
	return it;
}

bool ARKitFrameCorrelator::isComplete(const PendingEntry& entry) const
{
	const bool depthSatisfied= !m_depthStreamingEnabled.load() || entry.depth.has_value();
	return entry.hasVideo && entry.pose.has_value() && depthSatisfied;
}

ARKitFrameBundle ARKitFrameCorrelator::buildBundle(uint32_t frameSeq, const PendingEntry& entry) const
{
	ARKitFrameBundle bundle;
	bundle.frameSeq= frameSeq;

	// All three streams for the same frameSeq should carry the same
	// captureTimestampUs (same originating ARKit frame) - this priority order is
	// just a defensive fallback for whichever component actually showed up.
	if (entry.hasVideo)
		bundle.timestampUs= entry.videoTimestampUs;
	else if (entry.pose.has_value())
		bundle.timestampUs= entry.pose->captureTimestampUs;
	else if (entry.depth.has_value())
		bundle.timestampUs= entry.depth->captureTimestampUs;

	bundle.hasVideo= entry.hasVideo;
	bundle.videoFrameHandle= entry.videoFrameHandle;
	bundle.depth= entry.depth;
	bundle.pose= entry.pose;

	return bundle;
}

std::optional<ARKitFrameBundle> ARKitFrameCorrelator::tryFinalizeLocked(std::map<uint32_t, PendingEntry>::iterator it)
{
	if (!isComplete(it->second))
		return std::nullopt;

	ARKitFrameBundle bundle= buildBundle(it->first, it->second);
	m_pending.erase(it);
	++m_completedCount;
	return bundle;
}

void ARKitFrameCorrelator::deliverIfPresent(std::optional<ARKitFrameBundle>& bundle)
{
	if (!bundle.has_value())
		return;

	std::function<void(ARKitFrameBundle)> callbackCopy;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		callbackCopy= m_callback;
	}

	if (callbackCopy)
		callbackCopy(std::move(*bundle));
}

void ARKitFrameCorrelator::notifyVideoArrived(uint32_t frameSeq, uint64_t timestampUs, void* videoFrameHandle)
{
	std::optional<ARKitFrameBundle> bundle;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto it= getOrCreateLocked(frameSeq);
		it->second.hasVideo= true;
		it->second.videoFrameHandle= videoFrameHandle;
		it->second.videoTimestampUs= timestampUs;

		bundle= tryFinalizeLocked(it);
	}
	deliverIfPresent(bundle);
}

void ARKitFrameCorrelator::notifyDepthArrived(ARKitDepthFrame depth)
{
	const uint32_t frameSeq= depth.frameSeq;

	std::optional<ARKitFrameBundle> bundle;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto it= getOrCreateLocked(frameSeq);
		it->second.depth= std::move(depth);

		bundle= tryFinalizeLocked(it);
	}
	deliverIfPresent(bundle);
}

void ARKitFrameCorrelator::notifyPoseArrived(ARKitPoseFrame pose)
{
	const uint32_t frameSeq= pose.frameSeq;

	std::optional<ARKitFrameBundle> bundle;
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto it= getOrCreateLocked(frameSeq);
		it->second.pose= std::move(pose);

		bundle= tryFinalizeLocked(it);
	}
	deliverIfPresent(bundle);
}

void ARKitFrameCorrelator::sweepStaleFrames(std::chrono::steady_clock::time_point now)
{
	std::vector<ARKitFrameBundle> staleBundles;
	std::function<void(ARKitFrameBundle)> callbackCopy;

	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (auto it= m_pending.begin(); it != m_pending.end();)
		{
			if (now - it->second.firstArrival >= m_staleTimeout)
			{
				MIKAN_MT_LOG_WARNING("ARKitFrameCorrelator::sweepStaleFrames")
					<< "delivering partial bundle for frameSeq " << it->first << " (video=" << it->second.hasVideo
					<< " depth=" << it->second.depth.has_value() << " pose=" << it->second.pose.has_value() << ")";

				staleBundles.push_back(buildBundle(it->first, it->second));
				++m_partialDeliveredCount;
				it= m_pending.erase(it);
			}
			else
			{
				++it;
			}
		}

		callbackCopy= m_callback;
	}

	if (callbackCopy)
	{
		for (auto& bundle : staleBundles)
			callbackCopy(std::move(bundle));
	}
}

size_t ARKitFrameCorrelator::getPendingFrameCount() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_pending.size();
}

size_t ARKitFrameCorrelator::getCompletedFrameCount() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_completedCount;
}

size_t ARKitFrameCorrelator::getPartialDeliveredFrameCount() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_partialDeliveredCount;
}
