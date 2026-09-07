#pragma once

#include "ComponentFwd.h"
#include "InterprocessMessageServerInterface.h"
#include "MikanRendererFwd.h"

#include <cstdint>
#include <vector>

// Shared by the stencil and shape request handlers, which answer the same model render geometry
// question about two different component types.
//
// Serializing a model's triangle data walks every vertex through reflection, which is the bulk of
// the cost of answering the request. The payload only depends on the loaded meshes, so it is built
// once and cached on the render model resource, and the response is assembled by writing a fresh
// header in front of those cached bytes rather than re-serializing the whole thing.

// Returns the serialized MikanStencilModelRenderGeometry for these meshes. With a valid
// modelResource the payload is built once and cached on it; without one it is built into
// scratchPayload, which the caller must keep alive for as long as it uses the returned reference.
const std::vector<uint8_t>& fetchModelRenderGeometryPayload(
	MikanRenderModelResourcePtr modelResource, const std::vector<StaticMeshComponentPtr>& triMeshComponents,
	std::vector<uint8_t>& scratchPayload);

// Writes a successful model render geometry response carrying an already serialized geometry
// payload. The stencil and shape responses are the same struct under two names, so the wire type
// name is passed in rather than baked in.
void writeModelRenderGeometryResponse(const char* responseTypeName, MikanRequestID requestId,
									  const std::vector<uint8_t>& geometryPayload, ClientResponse& outResponse);

template <typename t_response_type>
void writeModelRenderGeometryResponse(MikanRequestID requestId, const std::vector<uint8_t>& geometryPayload,
									  ClientResponse& outResponse)
{
	writeModelRenderGeometryResponse(t_response_type::staticGetArchetype().getName(), requestId, geometryPayload,
									 outResponse);
}
