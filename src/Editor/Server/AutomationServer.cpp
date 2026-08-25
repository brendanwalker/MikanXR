#include "AutomationServer.h"
#include "App.h"
#include "AppStage.h"
#include "AutomationLogBuffer.h"
#include "AutomationProtocol.h"
#include "AutomationSocket.h"
#include "AutomationVariantText.h"
#include "CommonScriptContext.h"
#include "ComponentScriptContext.h"
#include "CompositorObjectSystem.h"
#include "FunctionDatabaseEnumerator.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MikanComponent.h"
#include "MikanFunctionDatabase.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MikanPropertyDatabase.h"
#include "MikanServer.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "PropertyDatabaseEnumerator.h"
#include "ScriptRequestHandler.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace
{
bool parseComponentId(const std::string& token, int& outComponentId)
{
	if (token.empty())
		return false;

	char* end= nullptr;
	const long parsed= strtol(token.c_str(), &end, 10);
	if (end == nullptr || *end != '\0')
		return false;

	outComponentId= (int)parsed;
	return true;
}

// Resolve a (system name, component id) command target.
// A component id of -1 targets the system itself (outComponent stays null).
bool resolveCommandTarget(MainWindow* mainWindow, const std::string& systemName, const std::string& componentIdToken,
						  MikanObjectSystemPtr& outSystem, MikanComponentPtr& outComponent, std::string& outError)
{
	outSystem= mainWindow->getProjectManager()->getSystemByName(systemName);
	if (!outSystem)
	{
		outError= "unknown system '" + systemName + "'";
		return false;
	}

	int componentId= -1;
	if (!parseComponentId(componentIdToken, componentId))
	{
		outError= "invalid component id '" + componentIdToken + "'";
		return false;
	}

	if (componentId != -1)
	{
		outComponent= outSystem->getComponentById(componentId);
		if (!outComponent)
		{
			outError= "no component with id " + std::to_string(componentId) + " in system '" + systemName + "'";
			return false;
		}
	}

	return true;
}
} // namespace

AutomationServer::AutomationServer()= default;

AutomationServer::~AutomationServer() { shutdown(); }

bool AutomationServer::startup(MainWindow* mainWindow, uint16_t port)
{
	m_mainWindow= mainWindow;

	registerCoreNamespaces();

	m_socket= std::make_unique<AutomationSocket>(port);
	m_socket->onLineReceived= [this](const std::string& line) { handleCommandLine(line); };

	if (!m_socket->isListening())
	{
		// A failed bind is tolerated: the editor runs fine without the
		// automation channel, it just cannot be driven externally
		m_socket= nullptr;
		return false;
	}

	return true;
}

void AutomationServer::poll()
{
	if (m_socket != nullptr)
	{
		m_socket->poll();
	}
}

void AutomationServer::shutdown()
{
	m_socket= nullptr;
	m_commandProviders.clear();
	m_mainWindow= nullptr;
}

void AutomationServer::registerCommandNamespace(const std::string& namespaceName,
												const std::vector<std::string>& helpLines, CommandHandler handler)
{
	m_commandProviders[namespaceName]= {helpLines, handler};
}

// ---- Dispatch ----------------------------------------------------------------

void AutomationServer::handleCommandLine(const std::string& line)
{
	std::vector<std::string> tokens;
	std::string parseError;
	if (!AutomationProtocol::tokenizeCommandLine(line, tokens, parseError))
	{
		sendErrorReply("parse: " + parseError);
		return;
	}

	// An empty line is a no-op that still answers, so a client stays in sync
	if (tokens.empty())
	{
		sendReply({});
		return;
	}

	const std::string namespaceName= tokens[0];
	auto providerIter= m_commandProviders.find(namespaceName);
	if (providerIter == m_commandProviders.end())
	{
		std::string validNames;
		for (const auto& [name, provider] : m_commandProviders)
		{
			if (!validNames.empty())
				validNames+= ", ";
			validNames+= name;
		}
		sendErrorReply("unknown command '" + namespaceName + "'; valid namespaces: " + validNames);
		return;
	}

	const std::vector<std::string> args(tokens.begin() + 1, tokens.end());
	std::vector<std::string> outLines;
	std::string outError;
	m_currentCommandLine= line;
	m_bReplyDeferred= false;
	if (providerIter->second.handler(args, outLines, outError))
	{
		if (!m_bReplyDeferred)
			sendReply(outLines);
	}
	else
	{
		std::string errorLine= namespaceName;
		if (!args.empty())
			errorLine+= " " + args[0];
		errorLine+= ": " + (outError.empty() ? std::string("failed") : outError);
		sendErrorReply(errorLine);
	}
}

void AutomationServer::sendReply(const std::vector<std::string>& contentLines)
{
	if (m_socket != nullptr)
	{
		m_socket->sendText(AutomationProtocol::frameReply(contentLines));
	}
}

void AutomationServer::sendErrorReply(const std::string& errorLine) { sendReply({errorLine}); }

// ---- Built-in command namespaces -----------------------------------------------

void AutomationServer::registerCoreNamespaces()
{
	using namespace std::placeholders;

	registerCommandNamespace("help", {"help"}, std::bind(&AutomationServer::handleHelpCommand, this, _1, _2, _3));

	registerCommandNamespace("app",
							 {"app info", "app push <stageName>", "app pop", "app open <projectPath>",
							  "app new <projectPath>", "app resume", "app quit"},
							 std::bind(&AutomationServer::handleAppCommand, this, _1, _2, _3));

	registerCommandNamespace("stage", {"stage <command> [parameters...]"},
							 std::bind(&AutomationServer::handleStageCommand, this, _1, _2, _3));

	registerCommandNamespace("system", {"system list"},
							 std::bind(&AutomationServer::handleSystemCommand, this, _1, _2, _3));

	registerCommandNamespace("component",
							 {"component list <system> [componentClass]", "component create <system> <componentClass>",
							  "component destroy <system> <componentId>"},
							 std::bind(&AutomationServer::handleComponentCommand, this, _1, _2, _3));

	registerCommandNamespace("property",
							 {"property list [system] [componentClass]", "property get <system> <componentId> <name>",
							  "property set <system> <componentId> <name> <value...>"},
							 std::bind(&AutomationServer::handlePropertyCommand, this, _1, _2, _3));

	registerCommandNamespace(
		"function", {"function list [system] [componentClass]", "function invoke <system> <componentId> <name>"},
		std::bind(&AutomationServer::handleFunctionCommand, this, _1, _2, _3));

	registerCommandNamespace("screenshot", {"screenshot compositor [componentId] [path]", "screenshot window [path]"},
							 std::bind(&AutomationServer::handleScreenshotCommand, this, _1, _2, _3));

	registerCommandNamespace("script",
							 {"script list", "script eval <system> <componentId> <lua-code>",
							  "script trigger <system> <componentId> <triggerName>"},
							 std::bind(&AutomationServer::handleScriptCommand, this, _1, _2, _3));

	registerCommandNamespace("log", {"log tail <lineCount> [trace|debug|info|warning|error|fatal]"},
							 std::bind(&AutomationServer::handleLogCommand, this, _1, _2, _3));

	// The history namespace is registered by TransactionHistory after startup
}

bool AutomationServer::handleHelpCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										 std::string& outError)
{
	for (const auto& [name, provider] : m_commandProviders)
	{
		for (const std::string& helpLine : provider.helpLines)
		{
			outLines.push_back(helpLine);
		}
	}

	return true;
}

bool AutomationServer::handleAppCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: app info|push|pop|open|new|resume|quit";
		return false;
	}

	const std::string& verb= args[0];
	AppStage* currentAppStage= m_mainWindow->getCurrentAppStage();

	if (verb == "info")
	{
		AppStage* parentAppStage= m_mainWindow->getParentAppStage();
		ProjectManagerPtr projectManager= m_mainWindow->getProjectManager();

		outLines.push_back("stage " + (currentAppStage != nullptr ? currentAppStage->getAppStageName() : "none"));
		outLines.push_back("parent " + (parentAppStage != nullptr ? parentAppStage->getAppStageName() : "none"));
		outLines.push_back("project "
						   + (projectManager->hasLoadedProject()
								  ? projectManager->getProjectConfig()->getLoadedConfigPath().string()
								  : "none"));
		return true;
	}
	else if (verb == "push")
	{
		if (args.size() < 2)
		{
			outError= "usage: app push <stageName>";
			return false;
		}

		const std::string& desiredStageName= args[1];
		if (currentAppStage != nullptr && currentAppStage->getAppStageName() == desiredStageName)
			return true;

		if (m_mainWindow->pushAppStage(desiredStageName) == nullptr)
		{
			outError= "unknown app stage '" + desiredStageName + "'";
			return false;
		}

		// The stage transition lands on the next frame (pending app stage ops)
		return true;
	}
	else if (verb == "pop")
	{
		if (m_mainWindow->getParentAppStage() == nullptr)
		{
			outError= "no parent app stage to pop to";
			return false;
		}

		m_mainWindow->popAppState();
		return true;
	}
	else if (verb == "open" || verb == "new" || verb == "resume")
	{
		// These route to the current stage's remote control commands,
		// which exist on the main menu stage
		static const std::map<std::string, std::string> k_stageCommands= {
			{"open", "open_project"}, {"new", "new_project"}, {"resume", "resume_project"}};

		std::vector<std::string> parameters(args.begin() + 1, args.end());
		std::vector<std::string> stageArgs= {k_stageCommands.at(verb)};
		stageArgs.insert(stageArgs.end(), parameters.begin(), parameters.end());

		return handleStageCommand(stageArgs, outLines, outError);
	}
	else if (verb == "quit")
	{
		App::getInstance()->requestShutdown();
		return true;
	}

	outError= "unknown verb '" + verb + "'";
	return false;
}

bool AutomationServer::handleStageCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										  std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: stage <command> [parameters...]";
		return false;
	}

	AppStage* currentAppStage= m_mainWindow->getCurrentAppStage();
	if (currentAppStage == nullptr)
	{
		outError= "no current app stage";
		return false;
	}

	const std::string& command= args[0];
	const std::vector<std::string> parameters(args.begin() + 1, args.end());

	std::vector<std::string> results;
	if (!currentAppStage->handleRemoteControlCommand(command, parameters, results))
	{
		outError= "command '" + command + "' not handled by stage '" + currentAppStage->getAppStageName() + "'";
		if (!results.empty())
			outError+= " (" + results[0] + ")";
		return false;
	}

	outLines= results;
	return true;
}

bool AutomationServer::handleSystemCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										   std::string& outError)
{
	if (args.empty() || args[0] != "list")
	{
		outError= "usage: system list";
		return false;
	}

	for (const MikanObjectSystemPtr& system : m_mainWindow->getProjectManager()->getSystems())
	{
		outLines.push_back(system->getObjectSystemClassName());
	}

	return true;
}

bool AutomationServer::handleComponentCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
											  std::string& outError)
{
	if (args.size() < 2)
	{
		outError= "usage: component list|create|destroy <system> ...";
		return false;
	}

	const std::string& verb= args[0];
	const std::string& systemName= args[1];
	ProjectManagerPtr projectManager= m_mainWindow->getProjectManager();
	MikanObjectSystemPtr objectSystem= projectManager->getSystemByName(systemName);
	if (!objectSystem)
	{
		outError= "unknown system '" + systemName + "'";
		return false;
	}

	if (verb == "create")
	{
		if (args.size() < 3)
		{
			outError= "usage: component create <system> <componentClass>";
			return false;
		}

		MikanComponentPtr component= objectSystem->addNewObjectWithDefaultDefinition(args[2]);
		if (!component)
		{
			outError= "system '" + systemName + "' cannot create a '" + args[2] + "'";
			return false;
		}

		outLines.push_back(std::to_string(component->getComponentId()));
		return true;
	}
	else if (verb == "destroy")
	{
		if (args.size() < 3)
		{
			outError= "usage: component destroy <system> <componentId>";
			return false;
		}

		int componentId= -1;
		if (!parseComponentId(args[2], componentId) || componentId == -1)
		{
			outError= "invalid component id '" + args[2] + "'";
			return false;
		}

		MikanComponentPtr component= objectSystem->getComponentById(componentId);
		if (!component)
		{
			outError= "no component with id " + std::to_string(componentId) + " in system '" + systemName + "'";
			return false;
		}

		if (!component->destroyOwnerObject())
		{
			outError= "failed to destroy component " + std::to_string(componentId);
			return false;
		}

		return true;
	}
	else if (verb != "list")
	{
		outError= "unknown verb '" + verb + "'";
		return false;
	}

	// Component classes come from the property database, which registers
	// every component class each system owns
	std::vector<std::string> componentClassNames;
	if (args.size() >= 3)
	{
		componentClassNames.push_back(args[2]);
	}
	else
	{
		for (const MikanPropertyEntry& entry : projectManager->getPropertyDatabaseConst()->getAllProperties())
		{
			if (entry.systemName != systemName || entry.componentClassName.empty())
				continue;

			if (std::find(componentClassNames.begin(), componentClassNames.end(), entry.componentClassName)
				== componentClassNames.end())
			{
				componentClassNames.push_back(entry.componentClassName);
			}
		}
	}

	for (const std::string& componentClassName : componentClassNames)
	{
		std::vector<int> componentIds;
		if (!objectSystem->getComponentIdList(componentClassName, componentIds))
			continue;

		for (int componentId : componentIds)
		{
			outLines.push_back(std::to_string(componentId) + " " + componentClassName);
		}
	}

	return true;
}

bool AutomationServer::handlePropertyCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
											 std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: property list|get|set ...";
		return false;
	}

	const std::string& verb= args[0];
	ProjectManagerPtr projectManager= m_mainWindow->getProjectManager();
	MikanPropertyDatabaseConstPtr propertyDatabase= projectManager->getPropertyDatabaseConst();

	if (verb == "list")
	{
		const std::string systemFilter= args.size() >= 2 ? args[1] : "";
		const std::string componentFilter= args.size() >= 3 ? args[2] : "";

		PropertyDatabaseEnumerator enumerator(propertyDatabase, systemFilter, componentFilter, "");
		while (enumerator.isValid())
		{
			const MikanPropertyEntry* entry= propertyDatabase->getPropertyByIndex(enumerator.getCurrentPropertyIndex());
			const std::string componentClassName= !entry->componentClassName.empty() ? entry->componentClassName : "-";

			outLines.push_back(entry->systemName + " " + componentClassName + " " + entry->descriptor->getName() + " "
							   + mikanVariantTypeToString(entry->descriptor->getDataType()) + " "
							   + (entry->descriptor->isReadOnly() ? "ro" : "rw"));

			enumerator.next();
		}

		return true;
	}
	else if (verb == "get" || verb == "set")
	{
		if (args.size() < 4)
		{
			outError=
				"usage: property " + verb + " <system> <componentId> <name>" + (verb == "set" ? " <value...>" : "");
			return false;
		}

		MikanObjectSystemPtr objectSystem;
		MikanComponentPtr component;
		if (!resolveCommandTarget(m_mainWindow, args[1], args[2], objectSystem, component, outError))
			return false;

		const std::string& propertyName= args[3];
		IPropertyInterface* propertyInterface= component != nullptr
												   ? static_cast<IPropertyInterface*>(component.get())
												   : static_cast<IPropertyInterface*>(objectSystem.get());

		if (verb == "get")
		{
			MikanVariant value;
			if (!propertyInterface->getPropertyValue(propertyName, value))
			{
				outError= "unknown property '" + propertyName + "'";
				return false;
			}

			outLines.push_back(AutomationVariantText::variantToText(value));
			return true;
		}
		else
		{
			if (args.size() < 5)
			{
				outError= "usage: property set <system> <componentId> <name> <value...>";
				return false;
			}

			// The descriptor supplies the target type for text coercion
			const std::string componentClassName= component != nullptr ? component->getComponentClassName() : "";
			PropertyDescriptorConstPtr descriptor=
				propertyDatabase->findPropertyDescriptor(args[1], componentClassName, propertyName);
			if (!descriptor)
			{
				outError= "unknown property '" + propertyName + "'";
				return false;
			}

			if (descriptor->isReadOnly())
			{
				outError= "property '" + propertyName + "' is read only";
				return false;
			}

			const std::vector<std::string> valueTokens(args.begin() + 4, args.end());
			MikanVariant value;
			if (!AutomationVariantText::textToVariant(descriptor->getDataType(), valueTokens, value, outError))
				return false;

			if (!propertyInterface->setPropertyValue(propertyName, value))
			{
				outError= "failed to set property '" + propertyName + "'";
				return false;
			}

			return true;
		}
	}

	outError= "unknown verb '" + verb + "'";
	return false;
}

bool AutomationServer::handleScreenshotCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
											   std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: screenshot compositor|window [path]";
		return false;
	}

	const std::string& verb= args[0];

	if (verb == "compositor")
	{
		// Optional compositor component id, then optional output path
		size_t nextArg= 1;
		int compositorId= -1;
		if (args.size() > nextArg && parseComponentId(args[nextArg], compositorId))
			++nextArg;
		const std::string path= args.size() > nextArg ? args[nextArg] : "mikan_compositor.png";

		auto compositorSystem= m_mainWindow->getProjectManager()->getSystemOfType<CompositorObjectSystem>();
		if (!compositorSystem)
		{
			outError= "no compositor system";
			return false;
		}

		if (compositorId == -1)
		{
			// Default to the project's single compositor; several need an explicit id
			std::vector<int> compositorIds;
			compositorSystem->getComponentIdList(CompositorComponent::k_componentClassName, compositorIds);
			if (compositorIds.size() != 1)
			{
				outError= "give a compositor component id (project has " + std::to_string(compositorIds.size())
						  + " compositors)";
				return false;
			}
			compositorId= compositorIds[0];
		}

		CompositorComponentPtr compositor= compositorSystem->getCompositorById(compositorId);
		if (!compositor)
		{
			outError= "no compositor with id " + std::to_string(compositorId);
			return false;
		}

		IMkTexturePtr frameTexture= compositor->getCompositedFrameTextureMutable();
		if (!frameTexture)
		{
			outError= "no composited frame available";
			return false;
		}

		if (!saveMkTextureToPNG(frameTexture, path.c_str()))
		{
			outError= "failed to write '" + path + "'";
			return false;
		}

		outLines.push_back(std::filesystem::absolute(path).string());
		return true;
	}
	else if (verb == "window")
	{
		if (m_bWindowCapturePending)
		{
			outError= "a window capture is already pending";
			return false;
		}

		// Park the capture until this frame's render completes; the reply is
		// sent from servicePendingWindowCapture
		m_windowCapturePath= args.size() >= 2 ? args[1] : "mikan_window.png";
		m_bWindowCapturePending= true;
		m_bReplyDeferred= true;
		return true;
	}

	outError= "unknown verb '" + verb + "'";
	return false;
}

void AutomationServer::servicePendingWindowCapture(int windowWidth, int windowHeight)
{
	if (!m_bWindowCapturePending)
		return;

	m_bWindowCapturePending= false;

	if (saveDefaultFramebufferToPNG(windowWidth, windowHeight, m_windowCapturePath.c_str()))
	{
		sendReply({std::filesystem::absolute(m_windowCapturePath).string()});
	}
	else
	{
		sendErrorReply("screenshot window: failed to write '" + m_windowCapturePath + "'");
	}
}

bool AutomationServer::handleScriptCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										   std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: script list|eval|trigger ...";
		return false;
	}

	const std::string& verb= args[0];

	if (verb == "list")
	{
		std::vector<CommonScriptContextPtr> scriptContexts;
		m_mainWindow->getMikanServer()->getScriptRequestHandler()->getBoundScriptContexts(scriptContexts);

		for (const CommonScriptContextPtr& scriptContext : scriptContexts)
		{
			// Component-owned contexts report their command target; other
			// contexts have no address and list as "- -1"
			std::string systemName= "-";
			int componentId= -1;
			auto componentContext= std::dynamic_pointer_cast<ComponentScriptContext>(scriptContext);
			if (componentContext)
			{
				MikanComponentPtr ownerComponent= componentContext->getOwnerComponent();
				if (ownerComponent)
				{
					systemName= ownerComponent->getOwnerObject()->getOwnerSystem()->getObjectSystemClassName();
					componentId= ownerComponent->getComponentId();
				}
			}

			std::string triggers;
			for (const std::string& trigger : scriptContext->getScriptTriggers())
			{
				if (!triggers.empty())
					triggers+= ",";
				triggers+= trigger;
			}

			outLines.push_back(systemName + " " + std::to_string(componentId) + " "
							   + scriptContext->getScriptFilename().string()
							   + (triggers.empty() ? "" : " " + triggers));
		}

		return true;
	}
	else if (verb == "eval" || verb == "trigger")
	{
		if (args.size() < 4)
		{
			outError= "usage: script " + verb + " <system> <componentId> "
					  + (verb == "eval" ? "<lua-code>" : "<triggerName>");
			return false;
		}

		MikanObjectSystemPtr objectSystem;
		MikanComponentPtr component;
		if (!resolveCommandTarget(m_mainWindow, args[1], args[2], objectSystem, component, outError))
			return false;

		if (!component)
		{
			outError= "script commands target a component, not a system";
			return false;
		}

		CommonScriptContextPtr scriptContext= component->getScriptContext();
		if (!scriptContext)
		{
			outError= "component has no script context";
			return false;
		}

		if (verb == "eval")
		{
			// Take the code as the raw untokenized tail of the command line,
			// so Lua quotes and spacing arrive verbatim
			const std::string code= AutomationProtocol::remainderAfterTokens(m_currentCommandLine, 4);

			std::string result;
			if (!scriptContext->evalString(code, result))
			{
				outError= result;
				return false;
			}

			if (!result.empty())
				outLines.push_back(result);
			return true;
		}
		else
		{
			const std::string& triggerName= args[3];
			if (!scriptContext->invokeScriptTrigger(triggerName))
			{
				outError= "trigger '" + triggerName + "' failed or does not exist";
				return false;
			}

			return true;
		}
	}

	outError= "unknown verb '" + verb + "'";
	return false;
}

bool AutomationServer::handleLogCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										std::string& outError)
{
	if (args.size() < 2 || args[0] != "tail")
	{
		outError= "usage: log tail <lineCount> [trace|debug|info|warning|error|fatal]";
		return false;
	}

	int lineCount= 0;
	if (!parseComponentId(args[1], lineCount) || lineCount <= 0)
	{
		outError= "invalid line count '" + args[1] + "'";
		return false;
	}

	int minLevel= 0;
	if (args.size() >= 3)
	{
		static const std::vector<std::string> k_levelNames= {"trace", "debug", "info", "warning", "error", "fatal"};
		const auto levelIter= std::find(k_levelNames.begin(), k_levelNames.end(), args[2]);
		if (levelIter == k_levelNames.end())
		{
			outError= "unknown log level '" + args[2] + "'";
			return false;
		}
		minLevel= (int)(levelIter - k_levelNames.begin());
	}

	AutomationLogBuffer::getTail(lineCount, minLevel, outLines);
	return true;
}

bool AutomationServer::handleFunctionCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
											 std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: function list|invoke ...";
		return false;
	}

	const std::string& verb= args[0];
	ProjectManagerPtr projectManager= m_mainWindow->getProjectManager();

	if (verb == "list")
	{
		const std::string systemFilter= args.size() >= 2 ? args[1] : "";
		const std::string componentFilter= args.size() >= 3 ? args[2] : "";
		MikanFunctionDatabaseConstPtr functionDatabase= projectManager->getFunctionDatabaseConst();

		FunctionDatabaseEnumerator enumerator(functionDatabase, systemFilter, componentFilter, "");
		while (enumerator.isValid())
		{
			const MikanFunctionEntry* entry= functionDatabase->getFunctionByIndex(enumerator.getCurrentFunctionIndex());
			const std::string componentClassName= !entry->componentClassName.empty() ? entry->componentClassName : "-";

			outLines.push_back(entry->systemName + " " + componentClassName + " " + entry->descriptor->getFunctionName()
							   + " " + entry->descriptor->getDisplayName());

			enumerator.next();
		}

		return true;
	}
	else if (verb == "invoke")
	{
		if (args.size() < 4)
		{
			outError= "usage: function invoke <system> <componentId> <name>";
			return false;
		}

		MikanObjectSystemPtr objectSystem;
		MikanComponentPtr component;
		if (!resolveCommandTarget(m_mainWindow, args[1], args[2], objectSystem, component, outError))
			return false;

		const std::string& functionName= args[3];
		const bool bInvoked=
			component != nullptr ? component->invokeFunction(functionName) : objectSystem->invokeFunction(functionName);
		if (!bInvoked)
		{
			outError= "unknown function '" + functionName + "'";
			return false;
		}

		return true;
	}

	outError= "unknown verb '" + verb + "'";
	return false;
}
