#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

#include "DeepNeuralNetwork.h"
#include "PathUtils.h"
#include "Logger.h"

#include <filesystem>
#include <vector>

#include <easy/profiler.h>

struct DNNData
{
	cv::dnn::Net net;
	int inputWidth;
	int inputHeight;
	int inputChannels;
	int outputWidth;
	int outputHeight;
	int outputChannels;
	int outputLayerId;
	std::string outputLayerName;
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
		return false;
	}

	// Find the first unconnected output layer
	std::vector<std::string> layerNames = net.getLayerNames();
	std::vector<int> unconnectedOutLayerIds= net.getUnconnectedOutLayers();
	if (unconnectedOutLayerIds.size() > 0)
	{
		m_dnnData->outputLayerId = unconnectedOutLayerIds[0];
		m_dnnData->outputLayerName = layerNames[m_dnnData->outputLayerId - 1];
	}
	else
	{
		MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") 
			<< "Unable to read output layer names from neural network: " << onnxPath.string();
		return false;
	}


	// Determine the shape of inputs required for the top layer
	std::vector<cv::dnn::MatShape> topLayerInputShapes;
	std::vector<cv::dnn::MatShape> topLayerOutputShapes;
	net.getLayerShapes(cv::dnn::MatShape(), 0, topLayerInputShapes, topLayerOutputShapes);

	if (topLayerInputShapes.size() == 0 || topLayerInputShapes[0].size() != 4)
	{
		MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") << 
			"Unable to read input layer shapes from neural network: " << onnxPath.string();
		return false;
	}
	else if (topLayerInputShapes[0][1] != 1 && topLayerInputShapes[0][1] != 3)
	{
		MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") <<
			"Input layer channel count unsupported size(" << topLayerInputShapes[0][1] <<
			"): " << onnxPath.string();
		return false;
	}

	//TODO: For some reason getLayerShapes isn't working for the output layer
	// Determine the shape of outputs returns from the bottom layer
	//std::vector<cv::dnn::MatShape> buttomLayerInputShapes;
	//std::vector<cv::dnn::MatShape> buttomLayerOutputShapes;
	//net.getLayerShapes(
	//	cv::dnn::MatShape(), 
	//	m_dnnData->outputLayerId, 
	//	buttomLayerInputShapes, buttomLayerOutputShapes);

	//if (buttomLayerOutputShapes.size() == 0 || buttomLayerOutputShapes[0].size() != 4)
	//{
	//	MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") << 
	//		"Unable to read output layer shapes from neural network: " << onnxPath.string();
	//	return false;
	//}		
	//else if (buttomLayerOutputShapes[0][1] != 1 && buttomLayerOutputShapes[0][1] != 3)
	//{
	//	MIKAN_LOG_ERROR("DeepNeuralNetwork::loadOnnxFile") <<
	//		"Output layer channel count unexpected size(" << topLayerOutputShapes[0][1] <<
	//		"): " << onnxPath.string();
	//	return false;
	//}

	// Example RGB input layer shape: [1, 3, 256, 256]
	m_dnnData->inputChannels = topLayerInputShapes[0][1];
	m_dnnData->inputWidth = topLayerInputShapes[0][2];
	m_dnnData->inputHeight = topLayerInputShapes[0][3];

	// Example Grayscale layer shape: [1, 1, 256, 256]
	//m_dnnData->outputChannels = buttomLayerOutputShapes[0][1];
	//m_dnnData->outputWidth = buttomLayerOutputShapes[0][2];
	//m_dnnData->outputHeight = buttomLayerOutputShapes[0][3];
	//TODO: For now, assume 1 channel output and same width and height as input
	m_dnnData->outputChannels = 1;
	m_dnnData->outputWidth = m_dnnData->inputWidth;
	m_dnnData->outputHeight = m_dnnData->inputHeight;

	// Tell OpenCV to use the GPU (otherwise evaluation will be super slow)
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

	m_dnnData->net = net;

	return true;
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

bool DeepNeuralNetwork::evaluateForwardPass(cv::Mat* inputBlob, cv::Mat* outputBlob) const
{
	EASY_FUNCTION();

	if (m_dnnData->net.empty())
		return false;

	if (inputBlob->dims != 4 || 
		inputBlob->size[3] != m_dnnData->inputWidth ||
		inputBlob->size[2] != m_dnnData->inputHeight ||
		inputBlob->size[1] != m_dnnData->inputChannels)
	{
		return false;
	}

	if (outputBlob->dims != 3 || 
		outputBlob->size[2] != m_dnnData->outputWidth ||
		outputBlob->size[1] != m_dnnData->outputHeight ||
		outputBlob->size[0] != m_dnnData->outputChannels)
	{
		return false;
	}

	if (m_dnnData->outputLayerName.empty())
	{
		return false;
	}

	// Forward pass of the blob through the neural network to get the predictions
	m_dnnData->net.setInput(*inputBlob);
	m_dnnData->net.forward(*outputBlob, m_dnnData->outputLayerName);

	return true;
}