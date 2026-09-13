#include "CompositorMaterialAssetReference.h"
#include "PathUtils.h"

// -- CompositorMaterialAssetReferenceFactory -----
CompositorMaterialAssetReferenceFactory::CompositorMaterialAssetReferenceFactory()
	: TypedAssetReferenceFactory<CompositorMaterialAssetReference, AssetReferenceConfig>()
{
	m_defaultPath=
		(PathUtils::getProjectDirectory() / MaterialDomainUtils::materialFolderName(eMaterialDomain::compositor) / "")
			.string();
}
