#include "ShapeMaterialAssetReference.h"
#include "PathUtils.h"

// -- ShapeMaterialAssetReferenceFactory -----
ShapeMaterialAssetReferenceFactory::ShapeMaterialAssetReferenceFactory()
	: TypedAssetReferenceFactory<ShapeMaterialAssetReference, AssetReferenceConfig>()
{
	m_defaultPath=
		(PathUtils::getProjectDirectory() / MaterialDomainUtils::materialFolderName(eMaterialDomain::shape) / "")
			.string();
}
