#include "CompositorGraphAssetReference.h"
#include "PathUtils.h"

// -- CompositorGraphAssetReferenceFactory -----
CompositorGraphAssetReferenceFactory::CompositorGraphAssetReferenceFactory()
	: TypedAssetReferenceFactory<CompositorGraphAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getProjectDirectory() / "compositors" / "").string();
}
