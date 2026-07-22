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
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_udp_receive_socket_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_depth_receiver_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_pose_receiver_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_frame_correlator_unit_tests);
	UNIT_TEST_SUITE_CALL_CPP_MODULE(run_arkit_video_device_interfaces_unit_tests);
	UNIT_TEST_SUITE_END()

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}