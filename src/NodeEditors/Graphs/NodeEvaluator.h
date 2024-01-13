#pragma once

#include "NodeFwd.h"

#include <memory>
#include <string>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

enum eNodeEvaluationErrorCode
{
	NONE= -1,

	invalidNode,
	missingInput,
	evaluationError,
	infiniteLoop
};

class NodeEvaluator
{
public:
	NodeEvaluator()= default;

	inline NodeEvaluator& setCurrentWindow(class IGlWindow* inWindow) { m_currentWindow= inWindow; return *this; }
	inline class IGlWindow* getCurrentWindow() const { return m_currentWindow; }

	inline NodeEvaluator& setCurrentVideoSourceView(VideoSourceViewPtr inVideoSourceView) 
	{ m_currentVideoSourceView= inVideoSourceView; return *this; }
	inline VideoSourceViewPtr getCurrentVideoSourceView() const 
	{ return m_currentVideoSourceView; }

	inline NodeEvaluator& setDeltaSeconds(float inDeltaSeconds) { m_deltaSeconds= inDeltaSeconds; return *this; }
	inline float getDeltaSeconds() const { return m_deltaSeconds; }

	inline void setLastErrorCode(eNodeEvaluationErrorCode errorCode) { m_errorCode= errorCode; }
	inline eNodeEvaluationErrorCode getLastErrorCode() const { return m_errorCode; }

	inline void setLastErrorMessage(const std::string& message) { m_errorMessage= message;  }
	inline const std::string& getLastErrorMessage() const { return m_errorMessage; }

	bool evaluateFlowPinChain(NodePtr startNode);

protected:
	class IGlWindow* m_currentWindow= nullptr;
	float m_deltaSeconds= 0.f;
	VideoSourceViewPtr m_currentVideoSourceView;

	NodePtr m_currentNode;
	eNodeEvaluationErrorCode m_errorCode;
	std::string m_errorMessage;
	int m_evaluatedNodeCount;

	static const int kInifiniteLoopThreshold= 1000;
};
