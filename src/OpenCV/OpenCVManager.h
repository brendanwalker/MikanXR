#pragma once

#include "OpenCVFwd.h"

#include <map>
#include <string>

//-- definitions -----
class OpenCVManager
{
public:
	OpenCVManager()= default;

	bool startup();
	void shutdown();

	inline bool supportsHardwareAcceleratedDNN() const { return m_bHasCUDA; }
	DeepNeuralNetworkPtr fetchDeepNeuralNetwork(const std::string& dnnFileName);

private:
	bool parseOpenCLBuildInfo();
	void parseOpenCVBuildInfo();

private:
	bool m_bHasCUDA = false;
	std::map<std::string, DeepNeuralNetworkPtr> m_dnnMap;
};
