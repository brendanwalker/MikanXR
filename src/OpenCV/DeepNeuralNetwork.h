#pragma once

#include "OpenCVFwd.h"

#include <filesystem>

class DeepNeuralNetwork
{
public:
	DeepNeuralNetwork();
	virtual ~DeepNeuralNetwork();

	static std::filesystem::path getOnnxFilePath(const std::string& onnx_filename);
	bool loadOnnxFile(const std::filesystem::path& onnxPath);
	void dispose();

	void setName(const std::string& name) { m_name = name; }
	const std::string getName() const { return m_name; }

	int getInputWidth() const;
	int getInputHeight() const;
	int getInputChannels() const;

	int getOutputWidth() const;
	int getOutputHeight() const;
	int getOutputChannels() const;

	bool evaluateForwardPass(cv::Mat* inputBlob, cv::Mat* outputBlob) const;

private:
	std::string m_name;
	struct DNNData* m_dnnData;
};