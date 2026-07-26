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

// A fully reassembled, RLE-decoded person-segmentation matte frame from the matte channel
// (basePort+3, see ARKitWireProtocol.h). Unlike depth (a fixed 256x192 LiDAR grid), the
// matte is the iPhone's NATIVE-resolution stencil, so its dimensions travel in the payload
// and vary by device. matte holds width*height 0/1 labels (0=not-person, 1=person),
// row-major.
struct ARKitMatteFrame
{
	uint32_t frameSeq= 0;
	uint64_t captureTimestampUs= 0;
	int width= 0;
	int height= 0;
	std::vector<uint8_t> matte;
};

// Reassembles matte channel UDP fragments (ARKitWireProtocol.h's matte fragment header +
// [uint32 width][uint32 height][uint32 rleLen][rle] payload) into complete, RLE-decoded
// ARKitMatteFrames. Pure logic, no sockets/threads - structurally identical to
// ARKitDepthFragmentAssembler (so its reassembly behavior is unit-testable without a live
// network); only the completed-payload parse differs (variable dims instead of a fixed
// 256x192 depth+confidence blob).
class ARKitMatteFragmentAssembler
{
public:
	explicit ARKitMatteFragmentAssembler(std::chrono::milliseconds staleTimeout= std::chrono::milliseconds(500));

	void setFrameCallback(std::function<void(ARKitMatteFrame)> callback);

	// Feed one raw matte-channel UDP datagram (fragment header + payload chunk). Returns
	// true if it was a well-formed, accepted fragment (even if incomplete or a harmless
	// duplicate), false if malformed/rejected. Never throws or reads out of bounds on
	// adversarial input. `now` is the receive-side wall clock for staleness bookkeeping.
	bool processFragment(const uint8_t* data, size_t length, std::chrono::steady_clock::time_point now);

	void sweepStaleFrames(std::chrono::steady_clock::time_point now);

	size_t getPendingFrameCount() const;
	size_t getDroppedStaleFrameCount() const;
	size_t getDroppedMalformedFrameCount() const;

private:
	struct FragmentAssemblyState
	{
		uint16_t fragCount= 0;
		uint64_t captureTimestampUs= 0;
		std::vector<bool> receivedMask;
		std::vector<std::vector<uint8_t>> fragmentPayloads;
		size_t receivedCount= 0;
		std::chrono::steady_clock::time_point firstArrival;
	};

	void tryCompleteFrame(uint32_t frameSeq);

	std::map<uint32_t, FragmentAssemblyState> m_pending;
	std::chrono::milliseconds m_staleTimeout;
	std::function<void(ARKitMatteFrame)> m_callback;
	size_t m_droppedStaleCount= 0;
	size_t m_droppedMalformedCount= 0;
};

// Owns a UdpReceiveSocket and a dedicated worker thread that feeds
// ARKitMatteFragmentAssembler, invoking the frame callback (from that worker thread)
// whenever a matte frame completes. Mirrors ARKitDepthReceiver.
class ARKitMatteReceiver
{
public:
	ARKitMatteReceiver();
	~ARKitMatteReceiver();

	ARKitMatteReceiver(const ARKitMatteReceiver&)= delete;
	ARKitMatteReceiver& operator=(const ARKitMatteReceiver&)= delete;

	bool start(uint16_t port);
	void stop();

	bool isRunning() const { return m_running.load(); }

	void setFrameCallback(std::function<void(ARKitMatteFrame)> callback);

	size_t getDroppedStaleFrameCount() const;
	size_t getDroppedMalformedFrameCount() const;

private:
	void workerThreadFunc();

	UdpReceiveSocket m_socket;
	ARKitMatteFragmentAssembler m_assembler;
	std::thread m_thread;
	std::atomic<bool> m_running{false};
};
