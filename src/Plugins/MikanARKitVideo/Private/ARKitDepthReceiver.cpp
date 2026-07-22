#include "ARKitDepthReceiver.h"
#include "ARKitWireProtocol.h"
#include "RVLCodec.h"
#include "Logger.h"

// -- ARKitDepthFragmentAssembler -----

ARKitDepthFragmentAssembler::ARKitDepthFragmentAssembler(std::chrono::milliseconds staleTimeout)
	: m_staleTimeout(staleTimeout)
{
}

void ARKitDepthFragmentAssembler::setFrameCallback(std::function<void(ARKitDepthFrame)> callback)
{
	m_callback= std::move(callback);
}

bool ARKitDepthFragmentAssembler::processFragment(const uint8_t* data, size_t length,
												  std::chrono::steady_clock::time_point now)
{
	ARKitDepthFragmentHeader header;
	if (!readARKitDepthFragmentHeader(data, length, header))
		return false;

	if (header.fragCount == 0 || header.fragIndex >= header.fragCount)
		return false;

	const uint8_t* fragmentPayload= data + ARKitDepthFragmentHeader::kWireSize;
	const size_t fragmentPayloadSize= length - ARKitDepthFragmentHeader::kWireSize;

	auto it= m_pending.find(header.frameSeq);
	if (it == m_pending.end())
	{
		FragmentAssemblyState state;
		state.fragCount= header.fragCount;
		state.captureTimestampUs= header.captureTimestampUs;
		state.receivedMask.assign(header.fragCount, false);
		state.fragmentPayloads.resize(header.fragCount);
		state.receivedCount= 0;
		state.firstArrival= now;

		it= m_pending.emplace(header.frameSeq, std::move(state)).first;
	}

	FragmentAssemblyState& state= it->second;

	// A fragCount mismatch against an already-in-progress assembly for this
	// frameSeq means this fragment is corrupted (or, astronomically unlikely, a
	// frameSeq reused after a full uint32 wraparound) - reject it without
	// disturbing the otherwise-valid in-progress state.
	if (state.fragCount != header.fragCount)
	{
		MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::processFragment")
			<< "frameSeq " << header.frameSeq << " fragCount mismatch (expected " << state.fragCount << ", got "
			<< header.fragCount << ") - rejecting fragment";
		return false;
	}

	// Duplicate fragment (retransmission/network duplication) - accepted as a
	// well-formed no-op, must not double-count toward fragCount.
	if (state.receivedMask[header.fragIndex])
		return true;

	state.fragmentPayloads[header.fragIndex].assign(fragmentPayload, fragmentPayload + fragmentPayloadSize);
	state.receivedMask[header.fragIndex]= true;
	++state.receivedCount;

	if (state.receivedCount == state.fragCount)
		tryCompleteFrame(header.frameSeq);

	return true;
}

void ARKitDepthFragmentAssembler::tryCompleteFrame(uint32_t frameSeq)
{
	auto it= m_pending.find(frameSeq);
	if (it == m_pending.end())
		return;

	FragmentAssemblyState& state= it->second;

	size_t totalSize= 0;
	for (const auto& fragment : state.fragmentPayloads)
		totalSize+= fragment.size();

	std::vector<uint8_t> payload;
	payload.reserve(totalSize);
	for (const auto& fragment : state.fragmentPayloads)
		payload.insert(payload.end(), fragment.begin(), fragment.end());

	// Parse [uint32 rvlLen][rvlBytes][uint32 confLen][confBytes], bounds-checking
	// every length against the actual reassembled payload before touching it - a
	// corrupted/adversarial length field must never drive an out-of-bounds read.
	constexpr size_t kLengthFieldSize= sizeof(uint32_t);

	if (payload.size() < kLengthFieldSize)
	{
		MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " payload too short for rvlLen header";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	size_t offset= 0;
	const uint32_t rvlLen= readU32BE(payload.data() + offset);
	offset+= kLengthFieldSize;

	if (rvlLen > payload.size() - offset)
	{
		MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " rvlLen " << rvlLen << " exceeds remaining payload";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	const uint8_t* rvlBytes= payload.data() + offset;
	offset+= rvlLen;

	if (payload.size() - offset < kLengthFieldSize)
	{
		MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " payload too short for confLen header";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	const uint32_t confLen= readU32BE(payload.data() + offset);
	offset+= kLengthFieldSize;

	if (confLen > payload.size() - offset)
	{
		MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " confLen " << confLen << " exceeds remaining payload";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	const uint8_t* confBytes= payload.data() + offset;

	std::vector<uint16_t> depthMM= rvlDecode(rvlBytes, rvlLen, kARKitDepthPixelCount);
	std::vector<uint8_t> confidence= packConfidenceRLEDecode(confBytes, confLen, kARKitDepthPixelCount);

	if (depthMM.empty() || confidence.empty())
	{
		MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " RVL/RLE decode failed";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	ARKitDepthFrame frame;
	frame.frameSeq= frameSeq;
	frame.captureTimestampUs= state.captureTimestampUs;
	frame.depthMM= std::move(depthMM);
	frame.confidence= std::move(confidence);

	if (m_callback)
		m_callback(std::move(frame));

	m_pending.erase(it);
}

void ARKitDepthFragmentAssembler::sweepStaleFrames(std::chrono::steady_clock::time_point now)
{
	for (auto it= m_pending.begin(); it != m_pending.end();)
	{
		if (now - it->second.firstArrival >= m_staleTimeout)
		{
			MIKAN_MT_LOG_WARNING("ARKitDepthFragmentAssembler::sweepStaleFrames")
				<< "dropping incomplete depth frame " << it->first << " (" << it->second.receivedCount << "/"
				<< it->second.fragCount << " fragments received)";
			++m_droppedStaleCount;
			it= m_pending.erase(it);
		}
		else
		{
			++it;
		}
	}
}

size_t ARKitDepthFragmentAssembler::getPendingFrameCount() const { return m_pending.size(); }

size_t ARKitDepthFragmentAssembler::getDroppedStaleFrameCount() const { return m_droppedStaleCount; }

size_t ARKitDepthFragmentAssembler::getDroppedMalformedFrameCount() const { return m_droppedMalformedCount; }

// -- ARKitDepthReceiver -----

ARKitDepthReceiver::ARKitDepthReceiver() {}

ARKitDepthReceiver::~ARKitDepthReceiver() { stop(); }

bool ARKitDepthReceiver::start(uint16_t port)
{
	stop();

	if (!m_socket.open(port))
		return false;

	m_running.store(true);
	m_thread= std::thread(&ARKitDepthReceiver::workerThreadFunc, this);
	return true;
}

void ARKitDepthReceiver::stop()
{
	m_running.store(false);
	m_socket.close(); // unblocks a receive() the worker thread may be mid-call in
	if (m_thread.joinable())
		m_thread.join();
}

void ARKitDepthReceiver::setFrameCallback(std::function<void(ARKitDepthFrame)> callback)
{
	m_assembler.setFrameCallback(std::move(callback));
}

size_t ARKitDepthReceiver::getDroppedStaleFrameCount() const { return m_assembler.getDroppedStaleFrameCount(); }

size_t ARKitDepthReceiver::getDroppedMalformedFrameCount() const { return m_assembler.getDroppedMalformedFrameCount(); }

void ARKitDepthReceiver::workerThreadFunc()
{
	// Fragments are capped at a ~1200-byte payload + 20-byte header (see
	// ARKitWireProtocol.h); sized with generous margin.
	uint8_t buffer[2048];

	while (m_running.load())
	{
		size_t bytesReceived= 0;
		UdpReceiveSocket::SenderAddress sender;

		if (m_socket.receive(buffer, sizeof(buffer), bytesReceived, sender))
		{
			m_assembler.processFragment(buffer, bytesReceived, std::chrono::steady_clock::now());
		}

		// receive()'s internal timeout (see UdpReceiveSocket) already bounds how
		// long this loop can go between iterations, giving sweepStaleFrames a
		// steady cadence to run on without a separate timer.
		m_assembler.sweepStaleFrames(std::chrono::steady_clock::now());
	}
}
