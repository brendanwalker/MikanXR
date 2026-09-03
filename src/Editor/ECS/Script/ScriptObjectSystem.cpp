#include "ScriptObjectSystem.h"
#include "Logger.h"
#include "MikanServer.h"
#include "PathUtils.h"
#include "ProjectScriptContext.h"
#include "ScriptRequestHandler.h"

#include <fstream>

// -- ScriptObjectSystemDefinition -----
ScriptObjectSystemDefinition::ScriptObjectSystemDefinition(const std::string& configName,
														   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- ScriptObjectSystem ----
ScriptObjectSystem::ScriptObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

void ScriptObjectSystem::postInit()
{
	Super::postInit();

	// Every other system has loaded by now, so scripts can resolve objects by name
	reloadAllScripts();
}

void ScriptObjectSystem::dispose()
{
	disposeScriptContext();

	Super::dispose();

	m_bReloadPending= false;
}

void ScriptObjectSystem::update(float deltaSeconds)
{
	if (m_bReloadPending)
	{
		reloadAllScripts();
	}

	Super::update(deltaSeconds);

	if (m_scriptContext)
	{
		m_scriptContext->updateScript(deltaSeconds);
	}
}

void ScriptObjectSystem::reloadAllScripts()
{
	m_bReloadPending= false;
	disposeScriptContext();

	// Gather the scripts with a path, in pool order
	std::vector<ScriptComponentPtr> scripts;
	for (ScriptDefinitionPtr definition : getTypedDefinitionConst()->getAllDefinitions())
	{
		ScriptComponentPtr script= getTypedComponentById(definition->getScriptId());
		if (script && definition->hasScriptPath())
		{
			scripts.push_back(script);
		}
	}

	// No state at all without scripts
	if (scripts.empty())
		return;

	m_scriptContext= std::make_shared<ProjectScriptContext>(getOwnerProjectManager());
	if (!m_scriptContext->createScriptState())
	{
		m_scriptContext= nullptr;
		return;
	}

	for (ScriptComponentPtr script : scripts)
	{
		const std::filesystem::path scriptPath= script->getResolvedScriptPath();
		if (!m_scriptContext->runScriptFile(scriptPath, script->getComponentId()))
		{
			// The failing chunk disposed the whole state
			MIKAN_LOG_ERROR("ScriptObjectSystem::reloadAllScripts")
				<< "Script " << script->getComponentId() << " (" << scriptPath << ") failed; project scripts unloaded";
			m_scriptContext= nullptr;
			return;
		}
	}

	MikanServer::getInstance()->getScriptRequestHandler()->bindScriptContext(m_scriptContext);
}

void ScriptObjectSystem::disposeScriptContext()
{
	if (!m_scriptContext)
		return;

	// Drop the HTTP routes and message subscription before the context goes away
	MikanServer* mikanServer= MikanServer::getInstance();
	if (mikanServer && mikanServer->getScriptRequestHandler())
	{
		mikanServer->getScriptRequestHandler()->unbindScriptContext(m_scriptContext);
	}

	m_scriptContext= nullptr;
}

ScriptComponentPtr ScriptObjectSystem::addNewScript()
{
	const std::filesystem::path scriptsDir= PathUtils::getProjectDirectory() / "scripts";
	std::filesystem::create_directories(scriptsDir);

	const std::filesystem::path scriptPath= PathUtils::makeTimestampedFilePath(scriptsDir, "script", ".lua");
	std::ofstream(scriptPath).flush();

	return addNewObjectByTypedDefinition(
		[&scriptPath](ScriptDefinitionPtr definition)
		{
			definition->setComponentName(scriptPath.stem().string());
			definition->setScriptPath(scriptPath);
			return true;
		});
}
