#include "OpenCVManager.h"
#include "DeepNeuralNetwork.h"
#include "Logger.h"

#include "opencv2/core.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/core/utils/logger.hpp"

#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/imgcodecs.hpp>

#include <filesystem>

static void setOpencvLoggingLevel(LogSeverityLevel logLevel)
{
	switch (logLevel)
	{
		case LogSeverityLevel::trace:
			cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);
			break;
		case LogSeverityLevel::debug:
			cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_DEBUG);
			break;
		case LogSeverityLevel::info:
			cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_INFO);
			break;
		case LogSeverityLevel::warning:
			cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
			break;
		case LogSeverityLevel::error:
			cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
			break;
		case LogSeverityLevel::fatal:
			cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
			break;
	}
}

bool OpenCVManager::startup()
{
	if (!parseOpenCLBuildInfo())
	{
		MIKAN_LOG_ERROR("OpenCVManager::init") << "Unable to initialize OpenCL";
		return false;
	}

	parseOpenCVBuildInfo();

	return true;
}

bool OpenCVManager::parseOpenCLBuildInfo()
{
	if (cv::ocl::haveOpenCL())
	{
		// Test for OpenCL availability
		cv::ocl::Device device = cv::ocl::Device::getDefault();

		const char* DeviceType = "Unknown";
		switch (device.type())
		{
			case cv::ocl::Device::TYPE_DEFAULT:
				DeviceType = "DEFAULT";
				break;
			case cv::ocl::Device::TYPE_CPU:
				DeviceType = "CPU";
				break;
			case cv::ocl::Device::TYPE_GPU:
				DeviceType = "GPU";
				break;
			case cv::ocl::Device::TYPE_ACCELERATOR:
				DeviceType = "ACCELERATOR";
				break;
			case cv::ocl::Device::TYPE_DGPU:
				DeviceType = "DGPU";
				break;
			case cv::ocl::Device::TYPE_IGPU:
				DeviceType = "IGPU";
				break;
			case cv::ocl::Device::TYPE_ALL:
				DeviceType = "ALL";
				break;
		}

		MIKAN_LOG_INFO("OpenCL") << "Device Name: " << device.name();
		MIKAN_LOG_INFO("OpenCL") << "Device Type: " << DeviceType;
		MIKAN_LOG_INFO("OpenCL") << "Device Vendor: " << device.vendorName();
		MIKAN_LOG_INFO("OpenCL") << "Device Version: " << device.version();
		MIKAN_LOG_INFO("OpenCL") << "OpenCL Version: " << device.OpenCLVersion();
		MIKAN_LOG_INFO("OpenCL") << "Has Kernel Compiler: " << (device.compilerAvailable() ? "YES" : "NO");
		MIKAN_LOG_INFO("OpenCL") << "Has Kernel Linker: " << (device.linkerAvailable() ? "YES" : "NO");

		// Set the log level in OpenCV
		setOpencvLoggingLevel(LogSeverityLevel::warning);
		return true;
	}

	return false;
}

// https://stackoverflow.com/questions/17347308/how-to-check-if-opencv-was-compiled-with-tbb-cuda-or-qt-support
void OpenCVManager::parseOpenCVBuildInfo()
{
	// Fetch full OpenCV build information report
	const cv::String str = cv::getBuildInformation();

	MIKAN_LOG_INFO("OpenCV") << "OpenCV Build Info";
	MIKAN_LOG_INFO("OpenCV") << "=================";

	// Parse the report line by line
	std::string line;
	std::istringstream strStream(str);
	while (std::getline(strStream, line))
	{
		// Enable this to see all the options. (Remember to remove the break)
		MIKAN_LOG_INFO("OpenCV") << line;

		if (line.find("cuDNN") != std::string::npos)
		{
			std::transform(line.begin(), line.end(), line.begin(), ::tolower);
			if (line.find("yes") != std::string::npos)
			{
				m_bHasCudaDNN = true;
				break;
			}
		}
	}
}

void OpenCVManager::shutdown()
{
	m_dnnMap.clear();
}

DeepNeuralNetworkPtr OpenCVManager::fetchDeepNeuralNetwork(const std::string& dnnFileName)
{
	auto it = m_dnnMap.find(dnnFileName);
	if (it != m_dnnMap.end())
	{
		return it->second;
	}
	else
	{
		auto dnn = std::make_shared<DeepNeuralNetwork>();

		std::filesystem::path dnnPath = DeepNeuralNetwork::getOnnxFilePath(dnnFileName);
		if (dnn->loadOnnxFile(dnnPath))
		{
			dnn->setName(dnnFileName);
			m_dnnMap.insert({dnnFileName, dnn});

			return dnn;
		}
	}

	return DeepNeuralNetworkPtr();
}