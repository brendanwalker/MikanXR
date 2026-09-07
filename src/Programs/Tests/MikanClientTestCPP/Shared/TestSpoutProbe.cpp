#include "TestSpoutProbe.h"
#include "Logger.h"

#include "SpoutDX.h"

#include <thread>

bool readSpoutSenderRgba(const std::string& senderName, std::vector<uint8_t>& outRgbaPixels, int& outWidth,
						 int& outHeight)
{
	spoutDX receiver;
	if (!receiver.OpenDirectX11())
	{
		MIKAN_LOG_ERROR("readSpoutSenderRgba") << "Failed to open a D3D11 device for the Spout receiver";
		return false;
	}
	receiver.SetReceiverName(senderName.c_str());

	// The first successful receive only reports the sender's size (IsUpdated). The reads after that
	// go through two staging textures, copying into one while mapping the other, so the pixels of a
	// read are the frame copied by the read before it: several reads are taken and the last kept.
	const int k_pixelReadsWanted= 4;
	int pixelReads= 0;
	std::vector<uint8_t> bgraPixels;
	for (int attempt= 0; attempt < 120 && pixelReads < k_pixelReadsWanted; ++attempt)
	{
		const unsigned int width= receiver.GetSenderWidth();
		const unsigned int height= receiver.GetSenderHeight();
		bgraPixels.resize((size_t)width * height * 4);

		if (receiver.ReceiveImage(bgraPixels.data(), width, height, false, false))
		{
			if (receiver.IsUpdated())
				continue;

			if (width > 0 && height > 0)
			{
				outWidth= (int)width;
				outHeight= (int)height;
				++pixelReads;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
	const bool bReceived= pixelReads > 0;

	// The color senders are BGRA and the packed depth sender RGBA, so the swizzle follows the format
	const DXGI_FORMAT senderFormat= receiver.GetSenderFormat();
	const bool bSenderIsBgra=
		senderFormat == DXGI_FORMAT_B8G8R8A8_UNORM || senderFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS
		|| senderFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || senderFormat == DXGI_FORMAT_B8G8R8X8_UNORM;

	receiver.ReleaseReceiver();
	receiver.CloseDirectX11();

	if (!bReceived)
	{
		MIKAN_LOG_ERROR("readSpoutSenderRgba") << "No frame received from Spout sender " << senderName;
		return false;
	}

	outRgbaPixels= bgraPixels;
	if (bSenderIsBgra)
	{
		for (size_t pixelIndex= 0; pixelIndex < bgraPixels.size() / 4; ++pixelIndex)
		{
			outRgbaPixels[pixelIndex * 4 + 0]= bgraPixels[pixelIndex * 4 + 2];
			outRgbaPixels[pixelIndex * 4 + 2]= bgraPixels[pixelIndex * 4 + 0];
		}
	}

	return true;
}
