#pragma once

#include "MikanAPI.h"

class MikanSpatialAnchorAPI : public IMikanSpatialAnchorAPI
{
public:
	MikanSpatialAnchorAPI() = default;
	MikanSpatialAnchorAPI(class MikanRequestManager* requestManager);

	inline static const std::string k_getSpatialAnchorList = "getSpatialAnchorList";
	virtual MikanResponseFuture getSpatialAnchorList() override;

	inline static const std::string k_getSpatialAnchorInfo = "getSpatialAnchorInfo";
	virtual MikanResponseFuture getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) override;

	inline static const std::string k_findSpatialAnchorInfoByName = "findSpatialAnchorInfoByName";
	virtual MikanResponseFuture findSpatialAnchorInfoByName(const std::string& anchorName) override;

private:
	class MikanRequestManager* m_requestManager = nullptr;
};
