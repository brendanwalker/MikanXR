#pragma once

#include <cuda.h>

#include "Logger.h"

// Shared CUDA Driver API error-checking helper for all of Track D's CUDA code
// (JBUKernel, and eventually D4's GL interop). Logs via Mikan's normal
// error-logging path and returns false - deliberately NOT exit()/abort() (unlike
// the CudaDepthUpsample prototype's CUDA_SAFE_CALL macro, which called exit(-1) on
// any error) and not a thrown exception either (ticket D3): CUDA errors here are
// expected-to-occur runtime conditions (driver hiccups, out-of-memory, an
// in-flight kernel faulting on bad input) that a long-running app must survive,
// not programming-error-only exceptional conditions. Callers treat a false return
// as "skip this frame's work", never as fatal.
inline bool checkCudaResult(CUresult result, const char* what)
{
	if (result == CUDA_SUCCESS)
		return true;

	const char* errorName= nullptr;
	const char* errorString= nullptr;
	cuGetErrorName(result, &errorName);
	cuGetErrorString(result, &errorString);

	MIKAN_LOG_ERROR("CUDA") << what << " failed: " << (errorName != nullptr ? errorName : "?") << " - "
							<< (errorString != nullptr ? errorString : "unknown error");

	return false;
}
