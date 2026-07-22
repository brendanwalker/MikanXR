//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "ARKitFrameCorrelator.h"
#include "unit_test.h"

namespace
{
ARKitDepthFrame makeSampleDepthFrame(uint32_t frameSeq)
{
	ARKitDepthFrame frame;
	frame.frameSeq= frameSeq;
	frame.captureTimestampUs= 100000ULL + frameSeq;
	frame.depthMM= {1, 2, 3}; // correlator only tracks presence, not content
	frame.confidence= {2, 2, 2};
	return frame;
}

ARKitPoseFrame makeSamplePoseFrame(uint32_t frameSeq)
{
	ARKitPoseFrame frame;
	frame.frameSeq= frameSeq;
	frame.captureTimestampUs= 200000ULL + frameSeq;
	return frame;
}

int dummyVideoHandleStorage= 0;
void* dummyVideoHandle() { return &dummyVideoHandleStorage; }
} // namespace

//-- private functions -----
static bool arkit_frame_correlator_test_completes_video_then_depth_then_pose()
{
	UNIT_TEST_BEGIN("completes when video, depth, pose arrive in that order")

	ARKitFrameCorrelator correlator(/*depthStreamingEnabled=*/true);
	int callbackCount= 0;
	ARKitFrameBundle received;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			++callbackCount;
			received= std::move(bundle);
		});

	correlator.notifyVideoArrived(1, 12345, dummyVideoHandle());
	success= (callbackCount == 0);
	assert(success);

	correlator.notifyDepthArrived(makeSampleDepthFrame(1));
	success= success && (callbackCount == 0);
	assert(success);

	correlator.notifyPoseArrived(makeSamplePoseFrame(1));
	success= success && (callbackCount == 1);
	assert(success);

	success= success && (received.frameSeq == 1);
	success= success && (received.hasVideo && received.videoFrameHandle == dummyVideoHandle());
	success= success && received.depth.has_value() && received.pose.has_value();
	assert(success);
	success= success && (correlator.getPendingFrameCount() == 0);
	success= success && (correlator.getCompletedFrameCount() == 1);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_completes_regardless_of_arrival_order()
{
	UNIT_TEST_BEGIN("completes exactly once regardless of notify order (all 6 permutations)")

	success= true;

	using NotifyFn= std::function<void(ARKitFrameCorrelator&, uint32_t)>;
	NotifyFn notifyVideo= [](ARKitFrameCorrelator& c, uint32_t seq)
	{ c.notifyVideoArrived(seq, 1, dummyVideoHandle()); };
	NotifyFn notifyDepth= [](ARKitFrameCorrelator& c, uint32_t seq)
	{ c.notifyDepthArrived(makeSampleDepthFrame(seq)); };
	NotifyFn notifyPose= [](ARKitFrameCorrelator& c, uint32_t seq) { c.notifyPoseArrived(makeSamplePoseFrame(seq)); };

	std::vector<std::vector<NotifyFn>> permutations= {
		{notifyVideo, notifyDepth, notifyPose}, {notifyVideo, notifyPose, notifyDepth},
		{notifyDepth, notifyVideo, notifyPose}, {notifyDepth, notifyPose, notifyVideo},
		{notifyPose, notifyVideo, notifyDepth}, {notifyPose, notifyDepth, notifyVideo},
	};

	uint32_t frameSeq= 100;
	for (const auto& order : permutations)
	{
		ARKitFrameCorrelator correlator(true);
		int callbackCount= 0;
		correlator.setBundleCallback([&](ARKitFrameBundle) { ++callbackCount; });

		for (const auto& step : order)
			step(correlator, frameSeq);

		success= (callbackCount == 1);
		assert(success);
		success= success && (correlator.getPendingFrameCount() == 0);
		assert(success);

		++frameSeq;
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_depth_disabled_completes_without_depth()
{
	UNIT_TEST_BEGIN("depth-disabled session completes on video+pose alone")

	ARKitFrameCorrelator correlator(/*depthStreamingEnabled=*/false);
	int callbackCount= 0;
	ARKitFrameBundle received;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			++callbackCount;
			received= std::move(bundle);
		});

	correlator.notifyVideoArrived(1, 1, dummyVideoHandle());
	success= (callbackCount == 0);
	assert(success);

	correlator.notifyPoseArrived(makeSamplePoseFrame(1));
	success= success && (callbackCount == 1);
	assert(success);

	success= success && !received.depth.has_value();
	success= success && received.pose.has_value() && received.hasVideo;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_depth_arriving_while_disabled_is_harmless()
{
	UNIT_TEST_BEGIN("a depth notification while streaming is disabled is stored but not required")

	ARKitFrameCorrelator correlator(false);
	int callbackCount= 0;
	ARKitFrameBundle received;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			++callbackCount;
			received= std::move(bundle);
		});

	// Depth shows up anyway (e.g. a race with a setting change) before video/pose.
	correlator.notifyDepthArrived(makeSampleDepthFrame(1));
	success= (callbackCount == 0);
	assert(success);

	correlator.notifyVideoArrived(1, 1, dummyVideoHandle());
	correlator.notifyPoseArrived(makeSamplePoseFrame(1));

	success= success && (callbackCount == 1);
	assert(success);
	// Depth was present when it completed, so it's opportunistically included.
	success= success && received.depth.has_value();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_stale_partial_missing_depth()
{
	UNIT_TEST_BEGIN("stale sweep delivers a partial bundle when only depth is missing")

	const auto staleTimeout= std::chrono::milliseconds(1000);
	ARKitFrameCorrelator correlator(true, staleTimeout);
	int callbackCount= 0;
	ARKitFrameBundle received;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			++callbackCount;
			received= std::move(bundle);
		});

	const auto start= std::chrono::steady_clock::now();
	correlator.notifyVideoArrived(1, 42, dummyVideoHandle());
	correlator.notifyPoseArrived(makeSamplePoseFrame(1));
	success= (callbackCount == 0); // still waiting on depth
	assert(success);

	// Not yet stale.
	correlator.sweepStaleFrames(start + staleTimeout - std::chrono::milliseconds(1));
	success= success && (callbackCount == 0);
	assert(success);
	success= success && (correlator.getPendingFrameCount() == 1);
	assert(success);

	// Now stale.
	correlator.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(1));
	success= success && (callbackCount == 1);
	assert(success);
	success= success && (correlator.getPendingFrameCount() == 0);
	assert(success);
	success= success && (correlator.getPartialDeliveredFrameCount() == 1);
	assert(success);

	success= success && received.hasVideo && received.pose.has_value() && !received.depth.has_value();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_stale_partial_pose_only()
{
	UNIT_TEST_BEGIN("stale sweep delivers pose alone when video/depth never arrive (pose is still useful)")

	const auto staleTimeout= std::chrono::milliseconds(100);
	ARKitFrameCorrelator correlator(true, staleTimeout);
	int callbackCount= 0;
	ARKitFrameBundle received;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			++callbackCount;
			received= std::move(bundle);
		});

	const auto start= std::chrono::steady_clock::now();
	correlator.notifyPoseArrived(makeSamplePoseFrame(7));

	correlator.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(1));
	success= (callbackCount == 1);
	assert(success);

	success= success && !received.hasVideo && received.videoFrameHandle == nullptr;
	success= success && !received.depth.has_value();
	success= success && received.pose.has_value() && received.pose->frameSeq == 7;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_stale_partial_video_only_missing_pose()
{
	UNIT_TEST_BEGIN("stale sweep delivers video-only when pose is lost")

	const auto staleTimeout= std::chrono::milliseconds(100);
	ARKitFrameCorrelator correlator(false, staleTimeout); // depth disabled to isolate the "missing pose" case
	int callbackCount= 0;
	ARKitFrameBundle received;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			++callbackCount;
			received= std::move(bundle);
		});

	const auto start= std::chrono::steady_clock::now();
	correlator.notifyVideoArrived(9, 1, dummyVideoHandle());

	correlator.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(1));
	success= (callbackCount == 1);
	assert(success);

	success= success && received.hasVideo && !received.pose.has_value() && !received.depth.has_value();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_fires_exactly_once_never_double_delivered()
{
	UNIT_TEST_BEGIN("a frameSeq is never delivered twice, whether completed or swept stale")

	const auto staleTimeout= std::chrono::milliseconds(50);
	ARKitFrameCorrelator correlator(true, staleTimeout);
	int callbackCount= 0;
	correlator.setBundleCallback([&](ARKitFrameBundle) { ++callbackCount; });

	const auto start= std::chrono::steady_clock::now();

	// Frame 1: completes naturally.
	correlator.notifyVideoArrived(1, 1, dummyVideoHandle());
	correlator.notifyDepthArrived(makeSampleDepthFrame(1));
	correlator.notifyPoseArrived(makeSamplePoseFrame(1));

	// Frame 2: goes stale (missing pose).
	correlator.notifyVideoArrived(2, 1, dummyVideoHandle());

	success= (callbackCount == 1); // only frame 1 so far
	assert(success);

	// Repeated sweeps after frame 2 is already gone must not re-deliver it.
	correlator.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(10));
	correlator.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(20));
	correlator.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(30));

	success= success && (callbackCount == 2);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_multiple_concurrent_frame_seqs_independent()
{
	UNIT_TEST_BEGIN("interleaved notifications for different frameSeqs correlate independently")

	ARKitFrameCorrelator correlator(true);
	std::vector<uint32_t> completedOrder;
	correlator.setBundleCallback([&](ARKitFrameBundle bundle) { completedOrder.push_back(bundle.frameSeq); });

	// Interleave: video(1), video(2), pose(2), depth(1), depth(2) [completes 2],
	// pose(1) [completes 1].
	correlator.notifyVideoArrived(1, 1, dummyVideoHandle());
	correlator.notifyVideoArrived(2, 1, dummyVideoHandle());
	correlator.notifyPoseArrived(makeSamplePoseFrame(2));
	correlator.notifyDepthArrived(makeSampleDepthFrame(1));
	correlator.notifyDepthArrived(makeSampleDepthFrame(2));
	success= (completedOrder.size() == 1 && completedOrder[0] == 2);
	assert(success);

	correlator.notifyPoseArrived(makeSamplePoseFrame(1));
	success= success && (completedOrder.size() == 2 && completedOrder[1] == 1);
	assert(success);

	success= success && (correlator.getPendingFrameCount() == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_frame_correlator_test_thread_safety_stress()
{
	UNIT_TEST_BEGIN("concurrent notify*() calls from multiple threads never crash, each frameSeq fires once")

	const uint32_t kFrameCount= 500;
	ARKitFrameCorrelator correlator(true, std::chrono::milliseconds(2000));

	std::mutex resultMutex;
	std::vector<uint32_t> deliveredSeqs;
	correlator.setBundleCallback(
		[&](ARKitFrameBundle bundle)
		{
			std::lock_guard<std::mutex> lock(resultMutex);
			deliveredSeqs.push_back(bundle.frameSeq);
		});

	std::thread videoThread(
		[&]
		{
			for (uint32_t seq= 0; seq < kFrameCount; ++seq)
				correlator.notifyVideoArrived(seq, 1, dummyVideoHandle());
		});
	std::thread depthThread(
		[&]
		{
			for (uint32_t seq= 0; seq < kFrameCount; ++seq)
				correlator.notifyDepthArrived(makeSampleDepthFrame(seq));
		});
	std::thread poseThread(
		[&]
		{
			for (uint32_t seq= 0; seq < kFrameCount; ++seq)
				correlator.notifyPoseArrived(makeSamplePoseFrame(seq));
		});

	videoThread.join();
	depthThread.join();
	poseThread.join();

	// Anything not naturally completed (lost the race across threads in some
	// interleaving) gets swept as partial - still exactly one delivery each.
	correlator.sweepStaleFrames(std::chrono::steady_clock::now() + std::chrono::seconds(5));

	std::lock_guard<std::mutex> lock(resultMutex);
	success= (deliveredSeqs.size() == kFrameCount);
	assert(success);

	std::set<uint32_t> uniqueSeqs(deliveredSeqs.begin(), deliveredSeqs.end());
	success= success && (uniqueSeqs.size() == kFrameCount); // no duplicates
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_frame_correlator_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_frame_correlator")
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_completes_video_then_depth_then_pose);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_completes_regardless_of_arrival_order);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_depth_disabled_completes_without_depth);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_depth_arriving_while_disabled_is_harmless);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_stale_partial_missing_depth);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_stale_partial_pose_only);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_stale_partial_video_only_missing_pose);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_fires_exactly_once_never_double_delivered);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_multiple_concurrent_frame_seqs_independent);
	UNIT_TEST_MODULE_CALL_TEST(arkit_frame_correlator_test_thread_safety_stress);
	UNIT_TEST_MODULE_END()
}
