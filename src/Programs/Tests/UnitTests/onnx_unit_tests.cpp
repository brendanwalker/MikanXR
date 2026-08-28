//-- includes -----
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "OnnxSession.h"
#include "unit_test.h"

//-- public interface -----
bool run_onnx_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("onnx")
	UNIT_TEST_MODULE_CALL_TEST(onnx_test_directml_probe);
	UNIT_TEST_MODULE_CALL_TEST(onnx_test_missing_model_fails_cleanly);
	UNIT_TEST_MODULE_END()
}

//-- private functions -----

// Reports whether DirectML is usable on this machine. This is INFORMATIONAL and
// never fails the suite: CI runners have no GPU, and OnnxSession falls back to
// the CPU provider. It exists so a machine that is supposed to have GPU
// inference says so out loud in the test log instead of silently running the
// lighting estimator on CPU at a fraction of the speed.
bool onnx_test_directml_probe()
{
	UNIT_TEST_BEGIN("directml availability probe")

	const bool bIsDirectMLAvailable= OnnxSession::isDirectMLAvailable();
	fprintf(stdout, "      DirectML execution provider: %s\n", bIsDirectMLAvailable ? "AVAILABLE" : "unavailable");

	// The probe must complete and report something either way.
	success= true;

	UNIT_TEST_COMPLETE()
}

bool onnx_test_missing_model_fails_cleanly()
{
	UNIT_TEST_BEGIN("missing model fails cleanly")

	OnnxSession session;

	// A model path that cannot exist must fail without throwing out of create()
	// and must leave the session in a well defined invalid state.
	success= !session.create("this_model_does_not_exist.onnx", "directml");
	assert(success);

	success&= !session.isValid();
	assert(success);

	success&= (strcmp(session.activeEp(), "none") == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}
