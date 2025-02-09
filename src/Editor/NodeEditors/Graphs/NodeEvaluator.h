#pragma once

#include "NodeFwd.h"
#include "NodeError.h"

#include <memory>
#include <string>
#include <vector>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class NodeEvaluator
{
public:
	NodeEvaluator()= default;

	inline NodeEvaluator& setCurrentWindow(class IMkWindow* inWindow) { m_currentWindow= inWindow; return *this; }
	inline class IMkWindow* getCurrentWindow() const { return m_currentWindow; }

	inline NodeEvaluator& setCurrentVideoSourceView(VideoSourceViewPtr inVideoSourceView) 
	{ m_currentVideoSourceView= inVideoSourceView; return *this; }
	inline VideoSourceViewPtr getCurrentVideoSourceView() const 
	{ return m_currentVideoSourceView; }

	inline NodeEvaluator& setDeltaSeconds(float inDeltaSeconds) { m_deltaSeconds= inDeltaSeconds; return *this; }
	inline float getDeltaSeconds() const { return m_deltaSeconds; }

	inline void setDisableInputEvaluation(bool bDisable) { m_bDisableInputEvaluation= bDisable; }
	inline bool getIsInputEvaluationDisabled() const { return m_bDisableInputEvaluation; }

	inline void addError(const NodeEvaluationError& error) { m_errors.push_back(error); }
	inline bool hasErrors() const { return !m_errors.empty(); }
	inline const std::vector<NodeEvaluationError>& getErrors() const { return m_errors; }

	bool evaluateFlowPinChain(NodePtr startNode);

protected:
	class IMkWindow* m_currentWindow= nullptr;
	float m_deltaSeconds= 0.f;
	VideoSourceViewPtr m_currentVideoSourceView;

	NodePtr m_currentNode;
	int m_evaluatedNodeCount= 0;
	std::vector<NodeEvaluationError> m_errors;
	bool m_bDisableInputEvaluation= false;

	static const int kInifiniteLoopThreshold= 1000;
};
