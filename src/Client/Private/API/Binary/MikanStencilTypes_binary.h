#pragma once

#include "MikanStencilTypes.h"
#include "BinaryUtility.h"

#include "MikanAPITypes_binary.h"
#include "MikanMathTypes_binary.h"

inline void to_binary(BinaryWriter& writer, const MikanTriagulatedMesh& triMesh)
{
	to_binary(writer, triMesh.vertices);
	to_binary(writer, triMesh.normals);
	to_binary(writer, triMesh.texels);
	to_binary(writer, triMesh.indices);
}
inline void from_binary(BinaryReader& reader, MikanTriagulatedMesh& triMesh)
{
	from_binary(reader, triMesh.vertices);
	from_binary(reader, triMesh.normals);
	from_binary(reader, triMesh.texels);
	from_binary(reader, triMesh.indices);
}

inline void to_binary(BinaryWriter& writer, const MikanStencilModelRenderGeometry& geo)
{
	const auto& response= (const MikanResponse&)geo;

	to_binary(writer, response);
	to_binary(writer, geo.meshes);
}
inline void from_binary(BinaryReader& reader, MikanStencilModelRenderGeometry& geo)
{
	auto& response = (MikanResponse&)geo;

	from_binary(reader, response);
	from_binary(reader, geo.meshes);
}