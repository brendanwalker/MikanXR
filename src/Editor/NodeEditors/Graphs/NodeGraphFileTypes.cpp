#include "NodeGraphFileTypes.h"

#include <string>

namespace NodeGraphFileTypes
{
bool isGraphFileExtension(const std::string& extension)
{
	return extension == k_compositorGraphExtension || extension == k_shapeGraphExtension
		   || extension == k_materialGraphExtension || extension == k_legacyGraphExtension;
}
} // namespace NodeGraphFileTypes
