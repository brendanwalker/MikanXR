#include "ARKitPoseReceiver.h"
#include "ARKitWireProtocol.h"
#include "Logger.h"

#include <algorithm>

// -- free functions -----

bool parseARKitPoseDatagram(const uint8_t* data, size_t length, ARKitPoseFrame& outFrame)
{
	ARKitPosePacket packet;
	if (!readARKitPosePacket(data, length, packet))
		return false;

	outFrame.frameSeq= packet.frameSeq;
	outFrame.captureTimestampUs= packet.captureTimestampUs;
	std::copy(std::begin(packet.transform), std::end(packet.transform), outFrame.transform);
	outFrame.fx= packet.fx;
	outFrame.fy= packet.fy;
	outFrame.cx= packet.cx;
	outFrame.cy= packet.cy;
	outFrame.imageWidth= packet.imageWidth;
	outFrame.imageHeight= packet.imageHeight;

	return true;
}

// -- ARKitPoseRingBuffer -----

ARKitPoseRingBuffer::ARKitPoseRingBuffer(size_t capacity)
	: m_capacity(capacity > 0 ? capacity : 1)
{
}

void ARKitPoseRingBuffer::push(ARKitPoseFrame frame)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	const uint32_t frameSeq= frame.frameSeq;
	auto existing= m_frames.find(frameSeq);
	if (existing != m_frames.end())
	{
		// Already-tracked frameSeq (e.g. a retransmitted duplicate) - update in
		// place, don't consume additional capacity or touch insertion order.
		existing->second= std::move(frame);
		return;
	}

	m_frames.emplace(frameSeq, std::move(frame));
	m_insertionOrder.push_back(frameSeq);

	if (m_insertionOrder.size() > m_capacity)
	{
		const uint32_t oldest= m_insertionOrder.front();
		m_insertionOrder.pop_front();
		m_frames.erase(oldest);
	}
}

bool ARKitPoseRingBuffer::tryGet(uint32_t frameSeq, ARKitPoseFrame& outFrame) const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	auto it= m_frames.find(frameSeq);
	if (it == m_frames.end())
		return false;

	outFrame= it->second;
	return true;
}

size_t ARKitPoseRingBuffer::size() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_frames.size();
}

// -- ARKitPoseReceiver -----

ARKitPoseReceiver::ARKitPoseReceiver(size_t ringBufferCapacity)
	: m_ringBuffer(ringBufferCapacity)
{
}

ARKitPoseReceiver::~ARKitPoseReceiver() { stop(); }

bool ARKitPoseReceiver::start(uint16_t port)
{
	stop();

	if (!m_socket.open(port))
		return false;

	m_running.store(true);
	m_thread= std::thread(&ARKitPoseReceiver::workerThreadFunc, this);
	return true;
}

void ARKitPoseReceiver::stop()
{
	m_running.store(false);
	m_socket.close(); // unblocks a receive() the worker thread may be mid-call in
	if (m_thread.joinable())
		m_thread.join();
}

void ARKitPoseReceiver::setFrameCallback(std::function<void(ARKitPoseFrame)> callback)
{
	m_callback= std::move(callback);
}

bool ARKitPoseReceiver::tryGetPose(uint32_t frameSeq, ARKitPoseFrame& outFrame) const
{
	return m_ringBuffer.tryGet(frameSeq, outFrame);
}

void ARKitPoseReceiver::workerThreadFunc()
{
	// Pose packets are a single unfragmented datagram, ARKitPosePacket::kWireSize
	// (104) bytes; sized with generous margin.
	uint8_t buffer[256];

	while (m_running.load())
	{
		size_t bytesReceived= 0;
		UdpReceiveSocket::SenderAddress sender;

		if (m_socket.receive(buffer, sizeof(buffer), bytesReceived, sender))
		{
			ARKitPoseFrame frame;
			if (parseARKitPoseDatagram(buffer, bytesReceived, frame))
			{
				m_ringBuffer.push(frame);

				if (m_callback)
					m_callback(std::move(frame));
			}
			else
			{
				++m_rejectedMalformedCount;
				MIKAN_MT_LOG_WARNING("ARKitPoseReceiver::workerThreadFunc")
					<< "rejected malformed pose datagram (" << bytesReceived << " bytes)";
			}
		}
	}
}
