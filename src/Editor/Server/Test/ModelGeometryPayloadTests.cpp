#include "ModelGeometryPayloadTests.h"
#include "unit_test.h"

#include "BinarySerializer.h"
#include "InterprocessMessageServerInterface.h"
#include "MikanShapeRequests.h"
#include "MikanStencilRequests.h"
#include "MikanStencilTypes.h"
#include "ServerModelGeometryPayload.h"

#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>

namespace
{
const MikanRequestID k_testRequestId= 4242;

// Two meshes with differing vertex counts, so a field written in the wrong order or a missing
// length prefix shifts the bytes rather than happening to line up.
MikanStencilModelRenderGeometry makeTestGeometry(int meshCount)
{
	MikanStencilModelRenderGeometry geometry= {};

	for (int meshIndex= 0; meshIndex < meshCount; ++meshIndex)
	{
		const int vertexCount= 3 + meshIndex;

		MikanTriagulatedMesh mesh= {};
		for (int i= 0; i < vertexCount; ++i)
		{
			const float f= (float)(meshIndex * 100 + i);

			mesh.vertices.push_back(MikanVector3f{f, f + 0.5f, f + 0.25f});
			mesh.normals.push_back(MikanVector3f{0.f, 1.f, 0.f});
			mesh.texels.push_back(MikanVector2f{f * 0.01f, 1.f - f * 0.01f});
			mesh.indices.push_back(i);
		}

		geometry.meshes.push_back(mesh);
	}

	return geometry;
}

// Serializes the response whole, then spliced, and reports whether the two agree.
template <typename t_response_type>
bool splicedResponseMatchesWholeResponse(int meshCount)
{
	t_response_type wholeResponse;
	wholeResponse.requestId= k_testRequestId;
	wholeResponse.resultCode= MikanAPIResult::Success;
	wholeResponse.render_geometry= makeTestGeometry(meshCount);

	std::vector<uint8_t> wholeBytes;
	std::string errorMsg;
	if (!Serialization::serializeToBytes(wholeResponse, wholeBytes, errorMsg))
	{
		fprintf(stdout, "      failed to serialize whole response: %s\n", errorMsg.c_str());
		return false;
	}

	std::vector<uint8_t> geometryBytes;
	if (!Serialization::serializeToBytes(wholeResponse.render_geometry, geometryBytes, errorMsg))
	{
		fprintf(stdout, "      failed to serialize geometry payload: %s\n", errorMsg.c_str());
		return false;
	}

	// A payload is never empty, even for a model with no meshes: the mesh count is always written.
	// fetchModelRenderGeometryPayload treats empty as "not built yet" and depends on this.
	if (geometryBytes.empty())
	{
		fprintf(stdout, "      geometry payload was unexpectedly empty\n");
		return false;
	}

	ClientResponse splicedResponse;
	writeModelRenderGeometryResponse<t_response_type>(k_testRequestId, geometryBytes, splicedResponse);

	if (splicedResponse.binaryData != wholeBytes)
	{
		fprintf(stdout, "      spliced response differs from whole response (%zu vs %zu bytes)\n",
				splicedResponse.binaryData.size(), wholeBytes.size());
		return false;
	}

	return true;
}
} // namespace

bool run_model_geometry_payload_tests()
{
	UNIT_TEST_MODULE_BEGIN("model_geometry_payload")
	UNIT_TEST_MODULE_CALL_TEST(model_geometry_payload_test_stencil_splice);
	UNIT_TEST_MODULE_CALL_TEST(model_geometry_payload_test_shape_splice);
	UNIT_TEST_MODULE_END()
}

bool model_geometry_payload_test_stencil_splice()
{
	UNIT_TEST_BEGIN("stencil response splices to the same bytes as a whole serialization")

	// No meshes, one mesh, and several, so the empty case is covered alongside the normal one
	success&= splicedResponseMatchesWholeResponse<MikanStencilModelRenderGeometryResponse>(0);
	assert(success);
	success&= splicedResponseMatchesWholeResponse<MikanStencilModelRenderGeometryResponse>(1);
	assert(success);
	success&= splicedResponseMatchesWholeResponse<MikanStencilModelRenderGeometryResponse>(3);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool model_geometry_payload_test_shape_splice()
{
	UNIT_TEST_BEGIN("shape response splices to the same bytes as a whole serialization")

	// Same struct as the stencil response under a different wire type name, so the header length
	// differs and a hardcoded offset would show up here
	success&= splicedResponseMatchesWholeResponse<MikanShapeModelRenderGeometryResponse>(0);
	assert(success);
	success&= splicedResponseMatchesWholeResponse<MikanShapeModelRenderGeometryResponse>(3);
	assert(success);

	UNIT_TEST_COMPLETE()
}
