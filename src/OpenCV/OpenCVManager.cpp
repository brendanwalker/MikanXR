#include "OpenCVManager.h"
#include "Logger.h"

#include "opencv2/core.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/core/utils/logger.hpp"

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
	bool success= true;

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
	}
	else
	{
		MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize OpenCL";
		success = false;
	}

	return success;
}

void OpenCVManager::shutdown()
{

}