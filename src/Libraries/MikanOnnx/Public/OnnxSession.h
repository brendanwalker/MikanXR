#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "onnxruntime_cxx_api.h"

// Thin wrapper around Ort::Session with DirectML-first execution provider
// selection and cached input/output metadata.
//
// Ported from MikanTrack (src/Vision/OnnxSession.h) - see NOTICE.md.
//
// THREAD AFFINITY: An OnnxSession instance must be created, run, and destroyed
// on a single thread. The shared Ort::Env outlives all sessions (it is a
// process-lifetime singleton).
class OnnxSession
{
public:
	OnnxSession()= default;
	~OnnxSession()= default;

	// Non-copyable
	OnnxSession(const OnnxSession&)= delete;
	OnnxSession& operator=(const OnnxSession&)= delete;

	// Loads the model at modelPath.
	// preferredEp: "directml" tries the DirectML EP (device 0) first with the
	// session options DML requires (mem pattern disabled, sequential
	// execution); on ANY failure it logs and falls back to a plain CPU
	// session. Anything else ("cpu", empty) goes straight to CPU.
	// Returns false if the model can't be loaded at all.
	bool create(const std::string& modelPath, const std::string& preferredEp);

	bool isValid() const { return m_session != nullptr; }

	// "DirectML" or "CPU" (or "none" before create succeeds)
	const char* activeEp() const { return m_activeEp.c_str(); }

	size_t getInputCount() const { return m_inputNames.size(); }
	size_t getOutputCount() const { return m_outputNames.size(); }
	const std::string& getInputName(size_t index) const { return m_inputNames[index]; }
	const std::string& getOutputName(size_t index) const { return m_outputNames[index]; }
	const std::vector<int64_t>& getInputShape(size_t index= 0) const { return m_inputShapes[index]; }
	const std::vector<int64_t>& getOutputShape(size_t index) const { return m_outputShapes[index]; }

	// Index of the first output whose last dimension equals lastDim, or -1.
	// Used to map outputs by shape rather than by graph order, since exporters
	// don't order them consistently across models.
	int findOutputByLastDim(int64_t lastDim) const;

	// Runs the session over all outputs. Inputs are in model input order.
	// Returns an empty vector if the run fails or was terminated.
	std::vector<Ort::Value> run(const Ort::Value* inputs, size_t inputCount);

	// Asks ONNX Runtime to abandon an in-flight run()/runOutputs() as soon as it
	// notices. The aborted call returns empty rather than throwing.
	//
	// THREAD SAFETY: this is the ONE method on this class that is safe to call
	// from a thread other than the session's owning thread - cancelling a
	// blocking Run from another thread is exactly what ORT's terminate flag is
	// for. The flag is sticky, so clearTerminate() must be called before the
	// session is usable again; run() does that itself at entry.
	void requestTerminate();
	void clearTerminate();

	// Runs the session over a SUBSET of outputs, returned in the order the
	// indices were given. ONNX Runtime prunes the graph to what was asked for,
	// so branches feeding only unrequested outputs never execute - which is
	// the difference between paying for a model's auxiliary heads and not.
	std::vector<Ort::Value> runOutputs(const Ort::Value* inputs, size_t inputCount, const int* outputIndices,
									   size_t outputIndexCount);

	// CPU memory info for building input tensors over preallocated buffers
	static const Ort::MemoryInfo& getCpuMemoryInfo();

	// True if the DirectML execution provider can be attached to a session on
	// this machine. Probes without loading a model, so it is cheap enough to
	// call from UI to report whether inference will be GPU or CPU backed.
	// A false result is not fatal - create() falls back to CPU - but it means
	// inference will be very slow.
	static bool isDirectMLAvailable();

private:
	static Ort::Env& getSharedEnv();

	void cacheIoMetadata();
	void logModelInfo(const std::string& modelPath) const;

	std::unique_ptr<Ort::Session> m_session;
	// Held rather than constructed per call so requestTerminate() has something
	// durable to set while a run is in flight.
	Ort::RunOptions m_runOptions;
	std::string m_activeEp= "none";

	std::vector<std::string> m_inputNames;
	std::vector<std::string> m_outputNames;
	std::vector<const char*> m_inputNamePtrs;
	std::vector<const char*> m_outputNamePtrs;
	std::vector<std::vector<int64_t>> m_inputShapes;
	std::vector<std::vector<int64_t>> m_outputShapes;
};
