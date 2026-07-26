#pragma once

// -- includes -----
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <thread>
#include <vector>

#include "UdpReceiveSocket.h"

// Depth channel (basePort+1) resolution, fixed by the wire protocol (see
// ARKitWireProtocol.h / RVLCodec.h) - matches ARKit's LiDAR sceneDepth resolution.
constexpr int kARKitDepthWidth= 256;
constexpr int kARKitDepthHeight= 192;
constexpr int kARKitDepthPixelCount= kARKitDepthWidth * kARKitDepthHeight;

// A fully reassembled, RVL/RLE-decoded depth frame ready for correlation with the
// matching video frame and pose packet (see ARKitFrameCorrelator, ticket B6).
struct ARKitDepthFrame
{
	uint32_t frameSeq= 0;
	uint64_t captureTimestampUs= 0;
	std::vector<uint16_t> depthMM;   // kARKitDepthPixelCount samples, millimeters, 0=invalid
	std::vector<uint8_t> confidence; // kARKitDepthPixelCount samples, 0/1/2
	// Optional person-segmentation matte (wire version >= 2). kARKitDepthPixelCount
	// samples, 0=not-person / 1=person. Empty when the sender didn't include a matte
	// (v1 stream, or matte disabled) - consumers must treat empty as "no gating".
	std::vector<uint8_t> segmentation;
};

// Reassembles depth channel UDP fragments (see ARKitWireProtocol.h's
// ARKitDepthFragmentHeader) into complete, RVL/RLE-decoded ARKitDepthFrames. Pure
// logic, no sockets/threads - this is what lets the reassembly behavior (in-order,
// out-of-order, duplicate, and adversarial fragment sequences) be unit tested
// directly, without a live network. ARKitDepthReceiver below wires this to a real
// UdpReceiveSocket on a worker thread.
class ARKitDepthFragmentAssembler
{
public:
	// staleTimeout: how long an incomplete frame is kept waiting for its remaining
	// fragments (tracked from the receive-side wall-clock arrival of that frame's
	// first fragment, via the explicit `now` parameter below - not the fragment's
	// own captureTimestampUs - and not by comparing frameSeq values against each
	// other, which sidesteps the uint32 frameSeq-wraparound hazard entirely rather
	// than needing wraparound-safe integer comparison).
	explicit ARKitDepthFragmentAssembler(std::chrono::milliseconds staleTimeout= std::chrono::milliseconds(500));

	void setFrameCallback(std::function<void(ARKitDepthFrame)> callback);

	// Feed one raw depth-channel UDP datagram (fragment header + payload chunk).
	// Returns true if it was a well-formed, accepted fragment (even if the frame it
	// belongs to isn't complete yet, or it was a harmless duplicate) - false if
	// malformed/rejected. Never throws or reads out of bounds on adversarial input.
	// `now` is the receive-side wall clock for staleness bookkeeping, passed in
	// explicitly (rather than read internally) so tests can simulate time passing
	// deterministically without actually sleeping.
	bool processFragment(const uint8_t* data, size_t length, std::chrono::steady_clock::time_point now);

	// Evicts any in-progress frame assemblies older than staleTimeout as of `now`.
	void sweepStaleFrames(std::chrono::steady_clock::time_point now);

	size_t getPendingFrameCount() const;
	size_t getDroppedStaleFrameCount() const;
	size_t getDroppedMalformedFrameCount() const;

private:
	struct FragmentAssemblyState
	{
		uint16_t fragCount= 0;
		uint8_t version= 0; // depth wire version from the frame's first fragment (>=2 carries a matte)
		uint64_t captureTimestampUs= 0;
		std::vector<bool> receivedMask;
		std::vector<std::vector<uint8_t>> fragmentPayloads;
		size_t receivedCount= 0;
		std::chrono::steady_clock::time_point firstArrival;
	};

	void tryCompleteFrame(uint32_t frameSeq);

	std::map<uint32_t, FragmentAssemblyState> m_pending;
	std::chrono::milliseconds m_staleTimeout;
	std::function<void(ARKitDepthFrame)> m_callback;
	size_t m_droppedStaleCount= 0;
	size_t m_droppedMalformedCount= 0;
};

// Owns a UdpReceiveSocket and a dedicated worker thread that feeds
// ARKitFragmentAssembler, invoking the frame callback (from that worker thread)
// whenever a depth frame completes.
class ARKitDepthReceiver
{
public:
	ARKitDepthReceiver();
	~ARKitDepthReceiver();

	ARKitDepthReceiver(const ARKitDepthReceiver&)= delete;
	ARKitDepthReceiver& operator=(const ARKitDepthReceiver&)= delete;

	// Opens the socket on `port` and starts the worker thread. Returns false (and
	// does not start a thread) if the port can't be bound - e.g. already in use.
	bool start(uint16_t port);

	// Stops the worker thread and closes the socket. Safe to call even if not
	// running; safe to call from a different thread than start()'s caller.
	void stop();

	bool isRunning() const { return m_running.load(); }

	// Invoked from the worker thread whenever a depth frame completes - the
	// callback must be safe to call from a background thread.
	void setFrameCallback(std::function<void(ARKitDepthFrame)> callback);

	size_t getDroppedStaleFrameCount() const;
	size_t getDroppedMalformedFrameCount() const;

private:
	void workerThreadFunc();

	UdpReceiveSocket m_socket;
	ARKitDepthFragmentAssembler m_assembler;
	std::thread m_thread;
	std::atomic<bool> m_running{false};
};
