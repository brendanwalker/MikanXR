#include "ServerModelGeometryPayload.h"
#include "BinarySerializer.h"
#include "Logger.h"
#include "MikanAPITypes.h"
#include "MikanRenderModelResource.h"
#include "MikanStencilTypes.h"
#include "StaticMeshComponent.h"

namespace
{
void buildGeometryPayload(const std::vector<StaticMeshComponentPtr>& triMeshComponents,
						  std::vector<uint8_t>& outPayload)
{
	MikanStencilModelRenderGeometry renderGeometry= {};
	for (StaticMeshComponentPtr mesh : triMeshComponents)
	{
		MikanTriagulatedMesh mikanMesh= {};
		mesh->extractRenderGeometry(mikanMesh);

		renderGeometry.meshes.push_back(mikanMesh);
	}

	outPayload.clear();

	std::string errorMsg;
	if (!Serialization::serializeToBytes(renderGeometry, outPayload, errorMsg))
	{
		MIKAN_LOG_ERROR("buildGeometryPayload") << "Failed to serialize model render geometry: " << errorMsg;
		outPayload.clear();
	}
}
} // namespace

const std::vector<uint8_t>& fetchModelRenderGeometryPayload(
	MikanRenderModelResourcePtr modelResource, const std::vector<StaticMeshComponentPtr>& triMeshComponents,
	std::vector<uint8_t>& scratchPayload)
{
	if (!modelResource)
	{
		buildGeometryPayload(triMeshComponents, scratchPayload);
		return scratchPayload;
	}

	// An empty payload means "not built yet": a model with no triangle meshes still serializes to a
	// zero mesh count, so a successfully built payload is never empty.
	if (modelResource->getClientGeometryPayload().empty())
	{
		std::vector<uint8_t> payload;
		buildGeometryPayload(triMeshComponents, payload);

		modelResource->setClientGeometryPayload(std::move(payload));
	}

	return modelResource->getClientGeometryPayload();
}

void writeModelRenderGeometryResponse(const char* responseTypeName, MikanRequestID requestId,
									  const std::vector<uint8_t>& geometryPayload, ClientResponse& outResponse)
{
	// Binary serialization writes a struct's fields in memory offset order, parents first, with no
	// framing between them. Serializing the response's base part and appending the already
	// serialized render_geometry therefore produces the same bytes as serializing the whole
	// response, without walking the geometry again. ModelGeometryPayloadTests pins that equality.
	MikanResponse header;
	header.responseTypeName= responseTypeName;
	header.requestId= requestId;
	header.resultCode= MikanAPIResult::Success;

	outResponse.binaryData.clear();

	std::string errorMsg;
	if (!Serialization::serializeToBytes(header, outResponse.binaryData, errorMsg))
	{
		MIKAN_LOG_ERROR("writeModelRenderGeometryResponse") << "Failed to serialize response header: " << errorMsg;
		outResponse.binaryData.clear();
		return;
	}

	outResponse.binaryData.insert(outResponse.binaryData.end(), geometryPayload.begin(), geometryPayload.end());
}
