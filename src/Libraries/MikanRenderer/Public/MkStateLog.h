#pragma once

#include "MkRendererFwd.h"
#include "IMkState.h"
#include "Logger.h"

class MkStateLog
{
public:
	MkStateLog(const IMkState* state)
		: m_loggerStream(LogSeverityLevel::info)
		, m_state(state) 
	{}

	template<class T>
	MkStateLog& operator<<(const T& x)
	{
		if (m_state->getOwnerStateStack().isDebugPrintEnabled())
		{
			m_loggerStream << m_state->getDebugPrefix() << x;
		}

		return *this;
	}

protected:
	LoggerStream m_loggerStream;
	const IMkState* m_state;
};