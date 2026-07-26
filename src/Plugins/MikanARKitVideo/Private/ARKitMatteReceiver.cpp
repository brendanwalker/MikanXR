#include "ARKitMatteReceiver.h"
#include "ARKitWireProtocol.h"
#include "RVLCodec.h"
#include "Logger.h"

// -- ARKitMatteFragmentAssembler -----

ARKitMatteFragmentAssembler::ARKitMatteFragmentAssembler(std::chrono::milliseconds staleTimeout)
	: m_staleTimeout(staleTimeout)
{
}

void ARKitMatteFragmentAssembler::setFrameCallback(std::function<void(ARKitMatteFrame)> callback)
{
	m_callback= std::move(callback);
}

bool ARKitMatteFragmentAssembler::processFragment(const uint8_t* data, size_t length,
												  std::chrono::steady_clock::time_point now)
{
	ARKitDepthFragmentHeader header;
	if (!readARKitMatteFragmentHeader(data, length, header))
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

	if (state.fragCount != header.fragCount)
	{
		MIKAN_MT_LOG_WARNING("ARKitMatteFragmentAssembler::processFragment")
			<< "frameSeq " << header.frameSeq << " fragCount mismatch (expected " << state.fragCount << ", got "
			<< header.fragCount << ") - rejecting fragment";
		return false;
	}

	if (state.receivedMask[header.fragIndex])
		return true;

	state.fragmentPayloads[header.fragIndex].assign(fragmentPayload, fragmentPayload + fragmentPayloadSize);
	state.receivedMask[header.fragIndex]= true;
	++state.receivedCount;

	if (state.receivedCount == state.fragCount)
		tryCompleteFrame(header.frameSeq);

	return true;
}

void ARKitMatteFragmentAssembler::tryCompleteFrame(uint32_t frameSeq)
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

	// Parse [uint32 width][uint32 height][uint32 rleLen][rle bytes], bounds-checking every
	// field against the actual reassembled payload before touching it.
	constexpr size_t kU32= sizeof(uint32_t);

	if (payload.size() < 3 * kU32)
	{
		MIKAN_MT_LOG_WARNING("ARKitMatteFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " payload too short for width/height/rleLen header";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	size_t offset= 0;
	const uint32_t width= readU32BE(payload.data() + offset);
	offset+= kU32;
	const uint32_t height= readU32BE(payload.data() + offset);
	offset+= kU32;
	const uint32_t rleLen= readU32BE(payload.data() + offset);
	offset+= kU32;

	// Reject implausible/adversarial dimensions before computing a pixel count that could
	// overflow or drive a huge allocation.
	if (width == 0 || height == 0 || width > static_cast<uint32_t>(kARKitMatteMaxDimension)
		|| height > static_cast<uint32_t>(kARKitMatteMaxDimension))
	{
		MIKAN_MT_LOG_WARNING("ARKitMatteFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " implausible matte dims " << width << "x" << height;
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	if (rleLen > payload.size() - offset)
	{
		MIKAN_MT_LOG_WARNING("ARKitMatteFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " rleLen " << rleLen << " exceeds remaining payload";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	const int pixelCount= static_cast<int>(width) * static_cast<int>(height);
	std::vector<uint8_t> matte= packConfidenceRLEDecode(payload.data() + offset, rleLen, pixelCount);

	if (matte.empty())
	{
		MIKAN_MT_LOG_WARNING("ARKitMatteFragmentAssembler::tryCompleteFrame")
			<< "frameSeq " << frameSeq << " matte RLE decode failed";
		++m_droppedMalformedCount;
		m_pending.erase(it);
		return;
	}

	ARKitMatteFrame frame;
	frame.frameSeq= frameSeq;
	frame.captureTimestampUs= state.captureTimestampUs;
	frame.width= static_cast<int>(width);
	frame.height= static_cast<int>(height);
	frame.matte= std::move(matte);

	if (m_callback)
		m_callback(std::move(frame));

	m_pending.erase(it);
}

void ARKitMatteFragmentAssembler::sweepStaleFrames(std::chrono::steady_clock::time_point now)
{
	for (auto it= m_pending.begin(); it != m_pending.end();)
	{
		if (now - it->second.firstArrival >= m_staleTimeout)
		{
			MIKAN_MT_LOG_WARNING("ARKitMatteFragmentAssembler::sweepStaleFrames")
				<< "dropping incomplete matte frame " << it->first << " (" << it->second.receivedCount << "/"
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

size_t ARKitMatteFragmentAssembler::getPendingFrameCount() const { return m_pending.size(); }

size_t ARKitMatteFragmentAssembler::getDroppedStaleFrameCount() const { return m_droppedStaleCount; }

size_t ARKitMatteFragmentAssembler::getDroppedMalformedFrameCount() const { return m_droppedMalformedCount; }

// -- ARKitMatteReceiver -----

ARKitMatteReceiver::ARKitMatteReceiver() {}

ARKitMatteReceiver::~ARKitMatteReceiver() { stop(); }

bool ARKitMatteReceiver::start(uint16_t port)
{
	stop();

	if (!m_socket.open(port))
		return false;

	m_running.store(true);
	m_thread= std::thread(&ARKitMatteReceiver::workerThreadFunc, this);
	return true;
}

void ARKitMatteReceiver::stop()
{
	m_running.store(false);
	m_socket.close();
	if (m_thread.joinable())
		m_thread.join();
}

void ARKitMatteReceiver::setFrameCallback(std::function<void(ARKitMatteFrame)> callback)
{
	m_assembler.setFrameCallback(std::move(callback));
}

size_t ARKitMatteReceiver::getDroppedStaleFrameCount() const { return m_assembler.getDroppedStaleFrameCount(); }

size_t ARKitMatteReceiver::getDroppedMalformedFrameCount() const { return m_assembler.getDroppedMalformedFrameCount(); }

void ARKitMatteReceiver::workerThreadFunc()
{
	// A native-resolution matte RLE's to tens of KB and thus spans many ~1200-byte
	// fragments; each individual datagram still fits this buffer with margin.
	uint8_t buffer[2048];

	while (m_running.load())
	{
		size_t bytesReceived= 0;
		UdpReceiveSocket::SenderAddress sender;

		if (m_socket.receive(buffer, sizeof(buffer), bytesReceived, sender))
		{
			m_assembler.processFragment(buffer, bytesReceived, std::chrono::steady_clock::now());
		}

		m_assembler.sweepStaleFrames(std::chrono::steady_clock::now());
	}
}
