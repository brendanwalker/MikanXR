#pragma once

#include "MikanAPITypes.h"
#include "MkRendererFwd.h"

#include <stdint.h>

struct ClientTextureFrameEntry
{
	IMkTexturePtr colorTexture;
	IMkTexturePtr depthTexture;
	int64_t frameIndex = -1;
};

class ClientTextureFrameQueue
{
public:
	explicit ClientTextureFrameQueue(int queueSize);
	~ClientTextureFrameQueue();

	bool initialize(const MikanRenderTargetDescriptor& desc);
	void dispose();

	// Current slot being written to — assign to SharedTextureReadAccessor
	IMkTexturePtr getPendingWriteColorTexture() const;
	IMkTexturePtr getPendingWriteDepthTexture() const;

	// Called from onClientRenderTargetUpdated: stamps frameIndex on current slot,
	// advances pointer. Caller must then re-assign getPendingWrite*() to the accessor.
	void advanceWriteIndex(int64_t frameIndex);

	// Lookup by frame index. Returns newest if frameIndex == -1 or not found.
	IMkTexturePtr getColorTexture(int64_t frameIndex = -1) const;
	IMkTexturePtr getDepthTexture(int64_t frameIndex = -1) const;

private:
	ClientTextureFrameEntry* m_entries = nullptr;
	int m_queueSize = 0;
	int m_lastWriteIndex = -1;
	int m_pendingWriteIndex = 0;
};
