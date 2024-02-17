#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

#include "DeepNeuralNetwork.h"
#include "PathUtils.h"
#include "Logger.h"

#include <filesystem>
#include <vector>

struct DNNData
{
	cv::dnn::Net net;
	int inputWidth;
	int inputHeight;
	int inputChannels;
	int outputWidth;
	int outputHeight;
	int outputChannels;
};

DeepNeuralNetwork::DeepNeuralNetwork()
	: m_dnnData(new DNNData())
{
}

DeepNeuralNetwork::~DeepNeuralNetwork()
{
	dispose();
	delete m_dnnData;
}

std::filesystem::path DeepNeuralNetwork::getOnnxFilePath(const std::string& onnx_filename)
{
	return PathUtils::getResourceDirectory() / std::string("dnn") / (onnx_filename + std::string(".onnx"));
}

bool DeepNeuralNetwork::loadOnnxFile(const std::filesystem::path& onnxPath)
{
	// Read in the neural network from the files
	auto net = cv::dnn::readNet(onnxPath.string());

	if (net.empty())
	{
		MIKAN_LOG_ERROR("OpenCVManager::fetchDeepNeuralNetwork") <<
			"Unable to read neural network from file: " << onnxPath.string();
	}
	else 
	{
		// Fetch the input and output layer shapes
		// we can use this to determine image buffer input and output sizes
		std::vector<cv::dnn::MatShape> inLayerShapes;
		std::vector<cv::dnn::MatShape> outLayerShapes;
		net.getLayerShapes(cv::dnn::MatShape(), 0, inLayerShapes, outLayerShapes);

		if (inLayerShapes.size() == 0 || inLayerShapes[0].size() != 4)
		{
			MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") << 
				"Unable to read input layer shapes from neural network: " << onnxPath.string();
		}
		else if (outLayerShapes.size() == 0 || outLayerShapes[0].size() != 4)
		{
			MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") << 
				"Unable to read output layer shapes from neural network: " << onnxPath.string();
		}
		else if (inLayerShapes[0][1] != 1 && inLayerShapes[0][1] != 3)
		{
			MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") <<
				"Input layer channel count unsupported size(" << inLayerShapes[0][1] <<
				"): " << onnxPath.string();
		}
		else if (outLayerShapes[0][1] != 1 && outLayerShapes[0][1] != 3)
		{
			MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") <<
				"Output layer channel count unexpected size(" << outLayerShapes[0][1] <<
				"): " << onnxPath.string();
		}
		else
		{
			// Example RGB input layer shape: [1, 3, 256, 256]
			m_dnnData->inputChannels = inLayerShapes[0][1];
			m_dnnData->inputWidth = inLayerShapes[0][2];
			m_dnnData->inputHeight = inLayerShapes[0][3];

			// Example Grayscale layer shape: [1, 1, 256, 256]
			m_dnnData->outputChannels = outLayerShapes[0][1];
			m_dnnData->outputWidth = outLayerShapes[0][2];
			m_dnnData->outputHeight = outLayerShapes[0][3];

			// Tell OpenCV to use the GPU (otherwise evaluation will be super slow)
			net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
			net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

			m_dnnData->net = net;
			return true;
		}
	}

	return false;
}

void DeepNeuralNetwork::dispose()
{
	m_dnnData->net = cv::dnn::Net();
	m_dnnData->inputChannels = 0;
	m_dnnData->inputWidth = 0;
	m_dnnData->inputHeight = 0;
	m_dnnData->outputChannels = 0;
	m_dnnData->outputWidth = 0;
	m_dnnData->outputHeight = 0;
}

int DeepNeuralNetwork::getInputWidth() const
{
	return m_dnnData->inputWidth;
}

int DeepNeuralNetwork::getInputHeight() const
{
	return m_dnnData->inputHeight;
}

int DeepNeuralNetwork::getInputChannels() const
{
	return m_dnnData->inputChannels;
}

int DeepNeuralNetwork::getOutputWidth() const
{
	return m_dnnData->outputWidth;
}

int DeepNeuralNetwork::getOutputHeight() const
{
	return m_dnnData->outputHeight;
}

int DeepNeuralNetwork::getOutputChannels() const
{
	return m_dnnData->outputChannels;
}