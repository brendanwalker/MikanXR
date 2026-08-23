#include "ScriptAssetReference.h"
#include "PathUtils.h"

// -- ScriptAssetReferenceFactory -----
ScriptAssetReferenceFactory::ScriptAssetReferenceFactory()
	: TypedAssetReferenceFactory<ScriptAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getProjectDirectory() / "scripts" / "").string();
}