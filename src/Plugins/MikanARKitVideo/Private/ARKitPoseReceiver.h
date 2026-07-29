#pragma once

// -- includes -----
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <thread>

#include "UdpReceiveSocket.h"

// A parsed pose channel (basePort+2) packet, ready for correlation with the
// matching video frame (see ARKitFrameCorrelator, ticket B6) or direct
// frame-indexed lookup (see IFrameCoupledPoseProvider, ticket E4).
struct ARKitPoseFrame
{
	uint32_t frameSeq= 0;
	uint64_t captureTimestampUs= 0;
	float transform[16]= {}; // row-major 4x4, camera-to-world
	float fx= 0.f;
	float fy= 0.f;
	float cx= 0.f;
	float cy= 0.f;
	float imageWidth= 0.f;
	float imageHeight= 0.f;
};

// Parses one raw pose-channel UDP datagram into an ARKitPoseFrame. Returns false on
// any malformed/truncated input (magic/version/type mismatch, undersized buffer) -
// exposed as a free function so it can be unit tested directly without a socket.
bool parseARKitPoseDatagram(const uint8_t* data, size_t length, ARKitPoseFrame& outFrame);

// Thread-safe fixed-capacity ring buffer of recently received pose frames, keyed by
// frameSeq for O(log n) point lookup. Capacity is small (pose packets are tiny,
// unfragmented, and should arrive promptly - there's no notion of an "incomplete"
// pose frame, just recent-enough-to-still-be-useful ones). Eviction is FIFO by
// insertion order, not by numeric frameSeq comparison - this sidesteps the uint32
// frameSeq-wraparound hazard entirely, at the accepted cost that a severely
// out-of-order late arrival could in principle evict a fresher entry; pose packets
// are expected to arrive promptly enough in practice that this is rare and
// low-consequence (a missed lookup just falls back gracefully downstream).
class ARKitPoseRingBuffer
{
public:
	explicit ARKitPoseRingBuffer(size_t capacity= 30);

	// Inserts or updates the entry for frame.frameSeq. Updating an already-tracked
	// frameSeq (e.g. a retransmitted duplicate) does not consume additional
	// capacity or evict anything.
	void push(ARKitPoseFrame frame);

	// Returns true and fills outFrame if frameSeq is currently in the buffer; false
	// (outFrame left untouched) if it was never received or has since been evicted.
	bool tryGet(uint32_t frameSeq, ARKitPoseFrame& outFrame) const;

	size_t size() const;

private:
	mutable std::mutex m_mutex;
	std::map<uint32_t, ARKitPoseFrame> m_frames;
	std::deque<uint32_t> m_insertionOrder;
	size_t m_capacity;
};

// Owns a UdpReceiveSocket and a dedicated worker thread that parses incoming pose
// packets and pushes them into an ARKitPoseRingBuffer, optionally also invoking a
// per-frame callback (from that worker thread) for consumers that want immediate
// push notification rather than later point-in-time lookup.
class ARKitPoseReceiver
{
public:
	explicit ARKitPoseReceiver(size_t ringBufferCapacity= 30);
	~ARKitPoseReceiver();

	ARKitPoseReceiver(const ARKitPoseReceiver&)= delete;
	ARKitPoseReceiver& operator=(const ARKitPoseReceiver&)= delete;

	// Opens the socket on `port` and starts the worker thread. Returns false (and
	// does not start a thread) if the port can't be bound - e.g. already in use.
	bool start(uint16_t port);

	// Stops the worker thread and closes the socket. Safe to call even if not
	// running; safe to call from a different thread than start()'s caller.
	void stop();

	bool isRunning() const { return m_running.load(); }

	// Invoked from the worker thread whenever a pose packet is received and parsed
	// successfully - the callback must be safe to call from a background thread.
	void setFrameCallback(std::function<void(ARKitPoseFrame)> callback);

	// Thread-safe point lookup, e.g. from a render/main thread looking up the pose
	// for a specific already-decoded video frame's frameSeq.
	bool tryGetPose(uint32_t frameSeq, ARKitPoseFrame& outFrame) const;

	size_t getRejectedMalformedCount() const { return m_rejectedMalformedCount.load(); }

private:
	void workerThreadFunc();

	UdpReceiveSocket m_socket;
	ARKitPoseRingBuffer m_ringBuffer;
	std::function<void(ARKitPoseFrame)> m_callback;
	std::thread m_thread;
	std::atomic<bool> m_running{false};
	std::atomic<size_t> m_rejectedMalformedCount{0};
};
