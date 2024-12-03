//-- includes -----
#include "SerializationProperty.h"
#include "Logger.h"

#include "MikanAPI.h"
#include "MikanClientEvents.h"
#include "MikanScriptEvents.h"
#include "MikanStencilEvents.h"
#include "MikanSpatialAnchorEvents.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVRDeviceEvents.h"
#include "MikanMathTypes.h"
#include "MikanScriptTypes.h"
#include "MikanStencilTypes.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVRDeviceTypes.h"

#include "Refureku/Refureku.h"

#include "stdio.h"

#include <algorithm>
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
	std::vector<rfk::Struct const*> serializableStructs;
	std::vector<rfk::Enum const*> enums;
};
using ClientModulePtr = std::shared_ptr<ClientModule>;

struct CodeGenDatabase
{
	std::map<std::string, ClientModulePtr> modules;

	void visitStruct(rfk::Struct const& entity)
	{
		ClientModulePtr module = findOrAddModule(entity);

		if (module != nullptr)
		{
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
			if (it != modules.end())
			{
				return it->second;
			}
			else
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

			if (codeGenDatabase.modules.size() > 0)
			{
				auto absOutputPath = std::filesystem::absolute(m_outputPath);

				// Nuke any previously generated code
				std::filesystem::remove_all(absOutputPath);

				// (Re)create the the output folder
				std::filesystem::create_directories(absOutputPath);

				for (auto const& module : codeGenDatabase.modules)
				{
					const std::string& moduleName = module.first;
					ClientModulePtr modulePtr = module.second;

					MIKAN_LOG_INFO("MikanClientCodeGen") << "Generate Code for Module: " << moduleName;
					if (!generateCodeForModule(absOutputPath, modulePtr))
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

		if (success)
		{
			MIKAN_LOG_INFO("MikanClientCodeGen") << "Working Directory: " << std::filesystem::current_path();
			MIKAN_LOG_INFO("MikanClientCodeGen") << "Loaded config: " << configPath;
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

	bool generateCodeForModule(
		const std::filesystem::path& absOutputPath,
		ClientModulePtr const& module)
	{
		std::string moduleFileName = module->name + ".cs";
		std::filesystem::path modulePath = absOutputPath / moduleFileName;

		try
		{
			std::ofstream moduleFile(modulePath);
			moduleFile << "// This file is auto generated. DO NO EDIT." << std::endl;
			moduleFile << "using System;" << std::endl;
			moduleFile << "using System.Collections.Generic;" << std::endl;
			moduleFile << std::endl;
			moduleFile << "namespace MikanXR" << std::endl;
			moduleFile << "{" << std::endl;

			emitModuleEntities(moduleFile, module);

			moduleFile << "}" << std::endl;

			moduleFile.close();
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
			std::sort(module->enums.begin(), module->enums.end(),
					  [](rfk::Enum const* a, rfk::Enum const* b) {
				return stricmp(a->getName(), b->getName()) < 0;
			});

			for (rfk::Enum const* enumRef : module->enums)
			{
				emitEnum(moduleFile, *enumRef);
				moduleFile << std::endl;
			}
		}

		if (module->serializableStructs.size() > 0)
		{
			std::sort(module->serializableStructs.begin(), module->serializableStructs.end(),
					  [](rfk::Struct const* a, rfk::Struct const* b) {
				return stricmp(a->getName(), b->getName()) < 0;
			});

			for (rfk::Struct const* structRef : module->serializableStructs)
			{
				emitSerializableClass(moduleFile, *structRef);
				moduleFile << std::endl;
			}
		}
	}

	void emitSerializableClass(std::ofstream& moduleFile, rfk::Struct const& structRef)
	{
		// Get a list of parent struct this struct inherits from
		using ParentList = std::vector<std::string>;
		ParentList parentStructNames;
		structRef.foreachDirectParent([](rfk::ParentStruct const& parentStruct, void* userData) -> bool {
			ParentList* parentStructNamesPtr = reinterpret_cast<ParentList*>(userData);
			std::string parentStructName = parentStruct.getArchetype().getName();

			parentStructNamesPtr->push_back(parentStructName);
			return true;
		}, &parentStructNames);

		// Generate struct inheritance string
		std::string structInheritance;
		if (parentStructNames.size() > 0)
		{
			structInheritance = " : ";
			for (size_t i = 0; i < parentStructNames.size(); ++i)
			{
				if (i > 0)
				{
					structInheritance += ", ";
				}
				structInheritance += parentStructNames[i];
			}
		}

		// Start of the struct definition
		moduleFile << "\tpublic class " << structRef.getName() << structInheritance << std::endl;
		moduleFile << "\t{" << std::endl;

		// Emit the refureku class id
		std::string classIdOverride = parentStructNames.size() > 0 ? "new " : "";
		moduleFile << "\t\tpublic static " << classIdOverride << "readonly ulong classId= " << structRef.getId() << ";" << std::endl;
		moduleFile << std::endl;

		// For some reason Refureku doesn't return fields in the order they were declared
		// So we extract fields into a vector and sort them by memory offset
		using FieldList = std::vector<rfk::Field const*>;
		FieldList sortedFields;
		structRef.foreachField([](rfk::Field const& field, void* userData) -> bool {
			FieldList* sortedFieldsPtr= reinterpret_cast<FieldList*>(userData);
			sortedFieldsPtr->push_back(&field);
			return true;
		}, &sortedFields);

		std::sort(sortedFields.begin(), sortedFields.end(), 
		[](rfk::Field const* a, rfk::Field const* b) {
			return a->getMemoryOffset() < b->getMemoryOffset();
		});

		// Emit the fields
		for (rfk::Field const* field : sortedFields)
		{
			std::string csharpType = getCSharpType(*field);
			moduleFile << "\t\tpublic " << csharpType << " " << field->getName() << ";" << std::endl;
		}

		// End of the struct definition
		moduleFile << "\t};" << std::endl;
	}

	void emitEnum(std::ofstream& moduleFile, rfk::Enum const& enumRef)
	{
		moduleFile << "\tpublic enum " << enumRef.getName() << std::endl;
		moduleFile << "\t{" << std::endl;

		enumRef.foreachEnumValue([](rfk::EnumValue const& enumValue, void* userData) -> bool {
			std::ofstream* moduleFilePtr = reinterpret_cast<std::ofstream*>(userData);

			// Only emit enum values with a string property
			auto const* property = enumValue.getProperty<Serialization::EnumStringValue>();
			if (property != nullptr)
			{
				const std::string enumValueString= property->getValue();
				const int64_t enumInt64Value = enumValue.getValue();

				(*moduleFilePtr) << "\t\t" << enumValueString << "= " << enumInt64Value << "," << std::endl;
			}

			return true;
		}, &moduleFile);

		moduleFile << "\t};" << std::endl;
	}

	static std::string getCSharpType(rfk::Field const& field)
	{
		rfk::Type const& fieldType = field.getType();

		return getCSharpType(fieldType);
	}

	static std::string getCSharpType(rfk::Type const& type)
	{
		rfk::Archetype const* archetype = type.getArchetype();
		rfk::EEntityKind fieldArchetypeKind = archetype ? archetype->getKind() : rfk::EEntityKind::Undefined;

		if (type.isPointer())
		{
			// All pointer types are IntPtr in C#
			return "IntPtr";
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Class)
		{
			rfk::Class const* classType = rfk::classCast(archetype);
			rfk::EClassKind classKind = classType->getClassKind();

			std::string cppType = classType->getName();

			if (cppType == "String")
			{
				return type.isCArray() ? "string[]" : "string";
			}
			else if (cppType == "BoolList")
			{
				return "List<bool>";
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				if (templateTypeName == "ObjectPtr" && 
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					auto const& templateArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& objectBaseType = templateArg.getType();
					std::string objectBaseTypeString= getCSharpType(objectBaseType);

					// Field type can be ObjectPtr template type since 
					// it's the base class for the pointer type
					return "SerializableObject<" + objectBaseTypeString + ">";
				}
				else if (templateTypeName == "List" &&
						 templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					auto const& templateArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& elementType = templateArg.getType();
					std::string elementTypeString= getCSharpType(elementType);

					return "List<" + elementTypeString + ">";
				}
				else if (templateTypeName == "Map" &&
						 templateClassInstanceType->getTemplateArgumentsCount() == 2)
				{
					auto const& templateKeyArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& keyType = templateKeyArg.getType();
					std::string keyTypeString= getCSharpType(keyType);

					auto const& templateValueArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(1));
					rfk::Type const& valueType = templateValueArg.getType();
					std::string valueTypeString= getCSharpType(valueType);

					return "Dictionary<" + keyTypeString + ", " + valueTypeString + ">";
				}
			}
			else
			{
				return type.isCArray() ? cppType+"[]" : cppType;
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(archetype);
			std::string structTypeName= structType->getName();

			return type.isCArray() ? structTypeName+"[]" : structTypeName;
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Enum)
		{
			rfk::Enum const* enumType = rfk::enumCast(archetype);
			std::string enumTypeName= enumType->getName();

			return type.isCArray() ? enumTypeName+"[]" : enumTypeName;
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::FundamentalArchetype)
		{
			std::string csType;

			if (type == rfk::getType<bool>())
			{
				csType= "bool";
			}
			else if (type == rfk::getType<uint8_t>())
			{
				csType= "byte";
			}
			else if (type == rfk::getType<int8_t>())
			{
				csType= "sbyte";
			}
			else if (type == rfk::getType<uint16_t>())
			{
				csType= "ushort";
			}
			else if (type == rfk::getType<int16_t>())
			{
				csType= "short";
			}
			else if (type == rfk::getType<uint32_t>())
			{
				csType= "uint";
			}
			else if (type == rfk::getType<int32_t>())
			{
				csType= "int";
			}
			else if (type == rfk::getType<uint64_t>())
			{
				csType= "ulong";
			}
			else if (type == rfk::getType<int64_t>())
			{
				csType= "long";
			}
			else if (type == rfk::getType<float>())
			{
				csType= "float";
			}
			else if (type == rfk::getType<double>())
			{
				csType= "double";
			}

			if (!csType.empty())
			{
				return type.isCArray() ? csType+"[]" : csType;
			}
			else
			{
				csType= "UNKNOWN_TYPE";
			}
		}

		return "UNKNOWN_TYPE";
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