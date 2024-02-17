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

	DeepNeuralNetworkPtr fetchDeepNeuralNetwork(const std::string& dnnFileName);

private:
	std::map<std::string, DeepNeuralNetworkPtr> m_dnnMap;
};
