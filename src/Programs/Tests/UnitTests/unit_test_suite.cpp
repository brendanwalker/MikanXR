//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include "unit_test.h"

//-- prototypes -----

//-- entry point -----
int main(int argc, char* argv[])
{
	UNIT_TEST_SUITE_BEGIN()
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_math_utility_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_math_glm_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_serialization_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_mikan_api_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_wire_protocol_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_rvl_codec_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_rvl_swift_crosscheck_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_rvl_real_capture_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_udp_receive_socket_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_depth_receiver_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_matte_receiver_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_pose_receiver_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_frame_correlator_unit_tests);
	// arkit_cuda_gl_interop must run BEFORE arkit_jbu_kernel, not after - confirmed
	// empirically that arkit_jbu_kernel's deliberate-CUDA-fault test (itself
	// already the last test within that module, for the same reason) leaves the
	// CUDA driver in a state where a later, completely independent cuCtxCreate()
	// call in this same process fails with CUDA_ERROR_ILLEGAL_ADDRESS - i.e. the
	// fault isn't cleanly scoped to just the one abandoned context, it can poison
	// this whole process's CUDA state. See project memory
	// (project_cuda_context_corruption_after_fault) for the full writeup. Any
	// future module that creates its own CUDA context must be registered before
	// arkit_jbu_kernel for this same reason.
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_cuda_gl_interop_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_jbu_kernel_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_video_device_interfaces_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_video_source_system_unit_tests);
	UNIT_TEST_SUITE_END()

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}