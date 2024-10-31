//-- includes -----
#include "SerializationProperty.h"
#include "Logger.h"

#include "MikanAPI.h"
#include "MikanEventTypes.h"
#include "MikanMathTypes.h"
#include "MikanScriptTypes.h"
#include "MikanSpatialAnchorTypes.h"
#include "MikanStencilTypes.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVRDeviceTypes.h"

#include "Refureku/Refureku.h"

#include "stdio.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#ifdef _MSC_VER
#pragma warning(disable:4996)  // ignore strncpy warning
#endif

struct ClientModule
{
	std::string name;
	std::vector<rfk::Class const*> apiClasses;
	std::vector<rfk::Struct const*> capiStructs;
	std::vector<rfk::Struct const*> serializableStructs;
	std::vector<rfk::Struct const*> events;
	std::vector<rfk::Struct const*> responses;
	std::vector<rfk::Enum const*> enums;
};
using ClientModulePtr = std::shared_ptr<ClientModule>;

struct CodeGenDatabase
{
	std::map<std::string, ClientModulePtr> modules;

	void visitClass(rfk::Class const& entity)
	{
		ClientModulePtr module= findOrAddModule(entity);

		if (module != nullptr)
		{
			module->apiClasses.push_back(&entity);
		}
	}

	void visitStruct(rfk::Struct const& entity)
	{
		ClientModulePtr module = findOrAddModule(entity);

		if (module != nullptr)
		{
			//events
			//responses
			//CAPIStructs
			module->serializableStructs.push_back(&entity);
		}
	}

	void visitEnum(rfk::Enum const& entity)
	{
		ClientModulePtr module = findOrAddModule(entity);

		if (module != nullptr)
		{
			module->enums.push_back(&entity);
		}
	}

	bool getEntityModuleName(rfk::Entity const& entity, std::string& outModuleName)
	{
		Serialization::CodeGenModule const* property = entity.getProperty<Serialization::CodeGenModule>();
		if (property != nullptr)
		{
			outModuleName= property->getModuleName();
			return true;
		}

		return false;
	}

	ClientModulePtr findOrAddModule(rfk::Entity const& entity)
	{
		std::string moduleName;

		if (getEntityModuleName(entity, moduleName))
		{
			auto it = modules.find(moduleName);
			if (it == modules.end())
			{
				ClientModulePtr module = std::make_shared<ClientModule>();
				module->name = moduleName;

				modules.insert({moduleName, module});
				return module;
			}
		}

		return ClientModulePtr();
	}
};

class MikanClientCodeGen
{
public:
	MikanClientCodeGen()
	{
	}

	virtual ~MikanClientCodeGen()
	{
		shutdown();
	}

	int exec(int argc, char** argv)
	{
		int result = 0;

		if (startup(argc, argv))
		{
			CodeGenDatabase codeGenDatabase;

			// Fetch all reflection data, sorted by module name
			fetchModules(codeGenDatabase);

			for (auto const& module : codeGenDatabase.modules)
			{
				const std::string& moduleName = module.first;
				ClientModulePtr modulePtr = module.second;

				if (moduleName == "MikanAPI")
				{
					MIKAN_LOG_INFO("MikanClientCodeGen") << "Generate Code for MikanAPI Module";
					if (!generateCodeForMikanAPIModule(modulePtr))
					{
						result = -1;
					}
				}
				else if (moduleName == "MikanCoreTypes")
				{
					MIKAN_LOG_INFO("MikanClientCodeGen") << "Generate Code for MikanCoreTypes Module";
					if (!generateCodeForMikanCoreTypesModule(modulePtr))
					{
						result = -1;
					}
				}
				else
				{
					MIKAN_LOG_INFO("MikanClientCodeGen") << "Generate Code for Module: " << moduleName;
					if (!generateCodeForModule(modulePtr))
					{
						result = -1;
					}
				}
			}
		}
		else
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to initialize!";
			result = -1;
		}

		shutdown();

		return result;
	}

protected:
	bool startup(int argc, char** argv)
	{
		bool success = true;

		LoggerSettings settings = {};
		settings.min_log_level = LogSeverityLevel::info;
		settings.enable_console = true;

		log_init(settings);

		if (argc < 2)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Usage: MikanClientCodeGen <path to config.json>";
			success= false;
		}

		std::filesystem::path configPath = argv[1];
		if (!std::filesystem::exists(configPath))
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Config file not found: " << configPath;
			success = false;
		}

		if (success)
		{
			success = parseConfig(configPath);
		}

		return success;
	}

	void shutdown()
	{
		log_dispose();
	}

	bool parseConfig(const std::filesystem::path& configPath)
	{
		std::string configPathString = configPath.string();
		MIKAN_LOG_INFO("MikanClientCodeGen") << "Loading config file: " << configPath;

		try
		{
			std::ifstream configFile(configPathString);
			std::stringstream configStream;
			configStream << configFile.rdbuf();
			std::string configString = configStream.str();

			json configJson = json::parse(configString);
			m_outputPath = (std::string)configJson["output_path"];
		}
		catch (const std::exception& e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to parse config file: " << e.what();
			return false;
		}

		return true;
	}

	void fetchModules(CodeGenDatabase& codeGenDatabase)
	{
		rfk::Database const& database= rfk::getDatabase();

		database.foreachFileLevelClass([](rfk::Class const& entity, void* userData) -> bool {
			auto* codeGenDB = reinterpret_cast<CodeGenDatabase*>(userData);
			codeGenDB->visitClass(entity);
			return true;
		}, &codeGenDatabase);

		database.foreachFileLevelStruct([](rfk::Struct const& entity, void* userData) -> bool {
			auto* codeGenDB = reinterpret_cast<CodeGenDatabase*>(userData);
			codeGenDB->visitStruct(entity);
			return true;
		}, &codeGenDatabase);

		database.foreachFileLevelEnum([](rfk::Enum const& entity, void* userData) -> bool {
			auto* codeGenDB = reinterpret_cast<CodeGenDatabase*>(userData);
			codeGenDB->visitEnum(entity);
			return true;
		}, &codeGenDatabase);
	}

	bool generateCodeForMikanAPIModule(ClientModulePtr const& module)
	{
		std::filesystem::path modulePath = std::filesystem::absolute(m_outputPath / "MikanCoreTypes.cs");

		try
		{
			std::ofstream moduleFile(modulePath);
			moduleFile << "// This file is auto generated. DO NO EDIT." << std::endl;
			moduleFile << "using System.Runtime.InteropServices;" << std::endl;
			moduleFile << std::endl;
			moduleFile << "namespace MikanXR" << std::endl;
			moduleFile << "{" << std::endl;

			emitModuleEntities(moduleFile, module);

			moduleFile << "}" << std::endl;

		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to write module file: " << modulePath;
			return false;
		}

		return true;
	}

	bool generateCodeForMikanCoreTypesModule(ClientModulePtr const& module)
	{
		std::filesystem::path modulePath = std::filesystem::absolute(m_outputPath / "MikanAPI.cs");

		try
		{
			std::ofstream moduleFile(modulePath);
			moduleFile << "// This file is auto generated. DO NO EDIT." << std::endl;
			moduleFile << "using Newtonsoft.Json;" << std::endl;
			moduleFile << "using System;" << std::endl;
			moduleFile << std::endl;
			moduleFile << "namespace MikanXR" << std::endl;
			moduleFile << "{" << std::endl;

			emitModuleEntities(moduleFile, module);

			moduleFile << "}" << std::endl;

		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to write module file: " << modulePath;
			return false;
		}

		return true;
	}

	bool generateCodeForModule(ClientModulePtr const& module)
	{
		std::string moduleFileName = module->name + ".cs";
		std::filesystem::path modulePath = std::filesystem::absolute(m_outputPath / moduleFileName);

		try
		{
			std::ofstream moduleFile(modulePath);
			moduleFile << "// This file is auto generated. DO NO EDIT." << std::endl;
			moduleFile << "using Newtonsoft.Json;" << std::endl;
			moduleFile << "using System;" << std::endl;
			moduleFile << std::endl;
			moduleFile << "namespace MikanXR" << std::endl;
			moduleFile << "{" << std::endl;

			emitModuleEntities(moduleFile, module);

			moduleFile << "}" << std::endl;

		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to write module file: " << modulePath;
			return false;
		}

		return true;
	}

	void emitModuleEntities(std::ofstream& moduleFile, ClientModulePtr const& module)
	{
		if (module->enums.size() > 0)
		{
			moduleFile << std::endl;
			moduleFile << "\t// Enums" << std::endl;
			for (rfk::Enum const* enumRef : module->enums)
			{
				emitEnum(moduleFile, *enumRef);
			}
		}

		if (module->serializableStructs.size() > 0)
		{
			moduleFile << std::endl;
			moduleFile << "\t// Structs" << std::endl;
			for (rfk::Struct const* structRef : module->serializableStructs)
			{
				emitSerializableStruct(moduleFile, *structRef);
			}
		}

		if (module->apiClasses.size() > 0)
		{
			moduleFile << std::endl;
			moduleFile << "\t// Classes" << std::endl;
			for (rfk::Class const* classRef : module->apiClasses)
			{
				emitApiClass(moduleFile, *classRef);
			}
		}
	}

	void emitApiClass(std::ofstream& moduleFile, rfk::Class const& classRef)
	{
		std::string className = classRef.getName();

		moduleFile << "\tpublic class " << className << std::endl;
		moduleFile << "\t{" << std::endl;

		moduleFile << "\t\tprivate MikanRequestManager _requestManager;" << std::endl;
		moduleFile << std::endl;
		moduleFile << "\t\tpublic " << className << "(MikanRequestManager requestManager)" << std::endl;
		moduleFile << "\t\t{" << std::endl;
		moduleFile << "\t\t\t_requestManager = requestManager;" << std::endl;
		moduleFile << "\t\t}" << std::endl;

		moduleFile << "\t};" << std::endl;
	}

	void emitMikanEvent(std::ofstream& moduleFile, rfk::Struct const& structRef)
	{
		std::string eventName = structRef.getName();

		moduleFile << "\tpublic class " << eventName << " : MikanEvent" << std::endl;
		moduleFile << "\t{" << std::endl;

		structRef.foreachField([](rfk::Field const& field, void* userData) -> bool {
			std::ofstream* moduleFilePtr = reinterpret_cast<std::ofstream*>(userData);
			std::string csharpType= getCSharpType(field.getType());

			(*moduleFilePtr) << "\t\tpublic " << csharpType << " " << field.getName() << " { get; set; }" << std::endl;
			return true;
		}, &moduleFile);

		moduleFile << "\t\tpublic " << eventName << "() : base(typeof(" << eventName << ").Name) {}" << std::endl;

		moduleFile << "\t};" << std::endl;
	}

	void emitMikanResponse(std::ofstream& moduleFile, rfk::Struct const& structRef)
	{
		std::string eventName = structRef.getName();

		moduleFile << "\tpublic class " << eventName << " : MikanEvent" << std::endl;
		moduleFile << "\t{" << std::endl;

		structRef.foreachField([](rfk::Field const& field, void* userData) -> bool {
			std::ofstream* moduleFilePtr = reinterpret_cast<std::ofstream*>(userData);
			std::string csharpType = getCSharpType(field.getType());

			(*moduleFilePtr) << "\t\tpublic " << csharpType << " " << field.getName() << " { get; set; }" << std::endl;
			return true;
		}, &moduleFile);

		moduleFile << "\t\tpublic " << eventName << "() : base(typeof(" << eventName << ").Name) {}" << std::endl;

		moduleFile << "\t};" << std::endl;
	}

	void emitSerializableStruct(std::ofstream& moduleFile, rfk::Struct const& structRef)
	{
		moduleFile << "\tpublic struct " << structRef.getName() << std::endl;
		moduleFile << "\t{" << std::endl;

		structRef.foreachField([](rfk::Field const& field, void* userData) -> bool {
			std::ofstream* moduleFilePtr = reinterpret_cast<std::ofstream*>(userData);
			std::string csharpType= getCSharpType(field.getType());

			(*moduleFilePtr) << "\t\tpublic " << csharpType << " " << field.getName() << " { get; set; }" << std::endl;
			return true;
		}, &moduleFile);

		moduleFile << "\t};" << std::endl;
	}

	void emitCAPIStruct(std::ofstream& moduleFile, rfk::Struct const& structRef)
	{
		moduleFile << "\t[StructLayout(LayoutKind.Sequential)]" << std::endl;
		moduleFile << "\tpublic struct " << structRef.getName() << std::endl;
		moduleFile << "\t{" << std::endl;

		structRef.foreachField([](rfk::Field const& field, void* userData) -> bool {
			std::ofstream* moduleFilePtr = reinterpret_cast<std::ofstream*>(userData);
			std::string csharpType = getCSharpType(field.getType());

			(*moduleFilePtr) << "\t\tpublic " << csharpType << " " << field.getName() << ";" << std::endl;
			return true;
		}, &moduleFile);

		moduleFile << "\t};" << std::endl;
	}

	void emitEnum(std::ofstream& moduleFile, rfk::Enum const& enumRef)
	{
		moduleFile << "\tpublic enum " << enumRef.getName() << std::endl;
		moduleFile << "\t{" << std::endl;

		enumRef.foreachEnumValue([](rfk::EnumValue const& enumValue, void* userData) -> bool {
			std::ofstream* moduleFilePtr = reinterpret_cast<std::ofstream*>(userData);
			(*moduleFilePtr) << "\t\t" << enumValue.getName() << "," << std::endl;
			return true;
		}, &moduleFile);

		moduleFile << "\t};" << std::endl;
	}

	static std::string getCSharpType(rfk::Type const& type)
	{
		std::string csharpType;

		const rfk::Archetype* archetype = type.getArchetype();
		if (archetype != nullptr)
		{
			std::string cppType = archetype->getName();

			if (cppType == "String")
			{
				csharpType= "string";
			}
		}

		return csharpType;
	}

private:
	std::filesystem::path m_outputPath;
};

//-- entry point -----
int main(int argc, char* argv[])
{
	IMikanAPIPtr apiPtr = IMikanAPI::createMikanAPI();
	MikanClientCodeGen app;

	return app.exec(argc, argv);
}