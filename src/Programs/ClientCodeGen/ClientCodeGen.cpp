//-- includes -----
#include "SerializationProperty.h"
#include "Logger.h"

#include "MikanAPI.h"
#include "MikanClientEvents.h"
#include "MikanScriptEvents.h"
#include "MikanVideoSourceEvents.h"
#include "MikanMathTypes.h"
#include "MikanScriptTypes.h"
#include "MikanStencilTypes.h"
#include "MikanCameraTypes.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVRDeviceTypes.h"

#include "Refureku/Refureku.h"

#include "stdio.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <ostream>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#ifdef _MSC_VER
#pragma warning(disable:4996)  // ignore strncpy warning
#endif

enum class TargetLanguage
{
	CSharp,
	TypeScript
};

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
				// Store reference to database for TypeScript import lookups
				m_codeGenDatabase = &codeGenDatabase;

				auto absOutputPath = std::filesystem::absolute(m_outputPath);

				// Nuke any previously generated code
				std::filesystem::remove_all(absOutputPath);

				// (Re)create the the output folder
				std::filesystem::create_directories(absOutputPath);

				// Generate code for all the modules we found
				result= generateCodeForModules(absOutputPath, codeGenDatabase) ? 0 : -1;

				m_codeGenDatabase = nullptr;
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

			// Parse target language (defaults to CSharp for backwards compatibility)
			if (configJson.contains("target_language"))
			{
				std::string targetLang = (std::string)configJson["target_language"];

				if (targetLang == "typescript")
				{
					m_targetLanguage = TargetLanguage::TypeScript;
				}
				else if (targetLang == "csharp")
				{
					m_targetLanguage = TargetLanguage::CSharp;
				}
				else 
				{
					throw std::runtime_error("Invalid 'target_language' field.");
				}
			}
			else
			{
				throw std::runtime_error("Config file missing 'target_language' field.");
			}
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

	bool generateCodeForModules(
		const std::filesystem::path& absOutputPath,
		CodeGenDatabase const& codeGenDatabase)
	{
		for (auto const& module : codeGenDatabase.modules)
		{
			const std::string& moduleName = module.first;
			ClientModulePtr modulePtr = module.second;

			MIKAN_LOG_INFO("MikanClientCodeGen") << "Generate Code for Module: " << moduleName;
			if (!generateCodeForModule(absOutputPath, modulePtr))
			{
				return false;
			}
		}

		if (m_targetLanguage == TargetLanguage::TypeScript)
		{
			if (!generateTypeScriptEnumRegistrationFile(absOutputPath, codeGenDatabase))
			{
				return false;
			}

			if (!generateTypeScriptIndexFile(absOutputPath, codeGenDatabase))
			{
				return false;
			}
		}

		return true;
	}

	bool generateTypeScriptIndexFile(
		const std::filesystem::path& absOutputPath,
		CodeGenDatabase const& codeGenDatabase)
	{
		std::filesystem::path indexFilePath = absOutputPath / "index.ts";

		try
		{
			std::ofstream moduleFile(indexFilePath);
			moduleFile << "// This file is auto generated. DO NO EDIT." << std::endl;
			moduleFile << "// Re-export all generated type modules" << std::endl;
			moduleFile << std::endl;

			for (auto const& module : codeGenDatabase.modules)
			{
				const std::string& moduleName = module.first;
				moduleFile << "export * from './" << moduleName << ".js';" << std::endl;
			}

			// Re-export the generated enum registration helper
			moduleFile << "export * from './EnumRegistration.js';" << std::endl;

			moduleFile.close();
		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to write index file: " << indexFilePath;
			return false;
		}

		return true;
	}

	bool generateTypeScriptEnumRegistrationFile(
		const std::filesystem::path& absOutputPath,
		CodeGenDatabase const& codeGenDatabase)
	{
		std::filesystem::path filePath = absOutputPath / "EnumRegistration.ts";

		// Build (moduleName -> [enumNames]) map
		std::map<std::string, std::vector<std::string>> moduleToEnums;
		std::vector<std::string> allEnumNames; // ordered for registration calls

		for (auto const& [moduleName, modulePtr] : codeGenDatabase.modules)
		{
			for (rfk::Enum const* enumRef : modulePtr->enums)
			{
				const std::string enumName = enumRef->getName();
				moduleToEnums[moduleName].push_back(enumName);
				allEnumNames.push_back(enumName);
			}
		}

		try
		{
			std::ofstream file(filePath);
			file << "// This file is auto generated. DO NOT EDIT." << std::endl;
			file << std::endl;
			file << "import { EnumRegistry } from '../Serialization/EnumRegistry.js';" << std::endl;

			// One import line per module that has enums
			for (auto const& [moduleName, enumNames] : moduleToEnums)
			{
				file << "import { ";
				for (size_t i = 0; i < enumNames.size(); ++i)
				{
					if (i > 0) file << ", ";
					file << enumNames[i];
				}
				file << " } from './" << moduleName << ".js';" << std::endl;
			}

			file << std::endl;
			file << "export function registerAllEnums(): void {" << std::endl;
			for (const std::string& enumName : allEnumNames)
			{
				file << "  EnumRegistry.register('" << enumName << "', " << enumName << ");" << std::endl;
			}
			file << "}" << std::endl;
			file << std::endl;
			file << "// Auto-register when this module is first imported." << std::endl;
			file << "// Because this file is re-exported from types/index.ts -> bindings/index.ts," << std::endl;
			file << "// this call runs automatically the moment any symbol from @mikanxr/client is imported." << std::endl;
			file << "registerAllEnums();" << std::endl;

			file.close();
		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to write EnumRegistration file: " << filePath;
			return false;
		}

		return true;
	}

	bool generateCodeForModule(
		const std::filesystem::path& absOutputPath,
		ClientModulePtr const& module)
	{
		if (m_targetLanguage == TargetLanguage::TypeScript)
		{
			return generateTypeScriptCodeForModule(absOutputPath, module);
		}
		else
		{
			return generateCSharpCodeForModule(absOutputPath, module);
		}
	}

	bool generateCSharpCodeForModule(
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

			emitCSharpModuleEntities(moduleFile, module);

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

	bool generateTypeScriptCodeForModule(
		const std::filesystem::path& absOutputPath,
		ClientModulePtr const& module)
	{
		std::string moduleFileName = module->name + ".ts";
		std::filesystem::path modulePath = absOutputPath / moduleFileName;

		try
		{
			std::ofstream moduleFile(modulePath);
			moduleFile << "// This file is auto generated. DO NOT EDIT." << std::endl;
			moduleFile << std::endl;

			emitTypeScriptModuleEntities(moduleFile, module);

			moduleFile.close();
		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("MikanClientCodeGen") << "Failed to write module file: " << modulePath;
			return false;
		}

		return true;
	}

	void emitCSharpModuleEntities(std::ofstream& moduleFile, ClientModulePtr const& module)
	{
		if (module->enums.size() > 0)
		{
			std::sort(module->enums.begin(), module->enums.end(),
					  [](rfk::Enum const* a, rfk::Enum const* b) {
				return stricmp(a->getName(), b->getName()) < 0;
			});

			for (rfk::Enum const* enumRef : module->enums)
			{
				emitCSharpEnum(moduleFile, *enumRef);
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
				emitCSharpSerializableClass(moduleFile, *structRef);
				moduleFile << std::endl;
			}
		}
	}

	void emitTypeScriptModuleEntities(std::ofstream& moduleFile, ClientModulePtr const& module)
	{
		// Collect import dependencies and group them by source module
		std::set<std::string> importedTypes;
		collectTypeScriptImports(module, importedTypes);

		// Build a map of module name -> list of types to import from that module
		std::map<std::string, std::vector<std::string>> moduleImports;

		for (const auto& typeName : importedTypes)
		{
			// Find which module this type belongs to by searching all modules in the database
			std::string sourceModuleName = findModuleForType(typeName);

			if (!sourceModuleName.empty())
			{
				moduleImports[sourceModuleName].push_back(typeName);
			}
		}

		// Emit imports grouped by module
		for (const auto& moduleImport : moduleImports)
		{
			const std::string& sourceModule = moduleImport.first;
			const std::vector<std::string>& types = moduleImport.second;

			moduleFile << "import { ";
			for (size_t i = 0; i < types.size(); ++i)
			{
				if (i > 0) moduleFile << ", ";
				moduleFile << types[i];
			}
			moduleFile << " } from './" << sourceModule << ".js';" << std::endl;
		}
		if (!moduleImports.empty())
		{
			moduleFile << std::endl;
		}

		// Emit enums
		if (module->enums.size() > 0)
		{
			std::sort(module->enums.begin(), module->enums.end(),
					  [](rfk::Enum const* a, rfk::Enum const* b) {
				return stricmp(a->getName(), b->getName()) < 0;
			});

			for (rfk::Enum const* enumRef : module->enums)
			{
				emitTypeScriptEnum(moduleFile, *enumRef);
				moduleFile << std::endl;
			}
		}

		// Emit class IDs as constants
		if (module->serializableStructs.size() > 0)
		{
			std::sort(module->serializableStructs.begin(), module->serializableStructs.end(),
					  [](rfk::Struct const* a, rfk::Struct const* b) {
				return stricmp(a->getName(), b->getName()) < 0;
			});

			for (rfk::Struct const* structRef : module->serializableStructs)
			{
				size_t classId = structRef->getId();
				int64_t classIdInt64 = *reinterpret_cast<int64_t*>(&classId);
				std::string constName = "CLASS_ID_" + toUpperSnakeCase(structRef->getName());
				moduleFile << "export const " << constName << " = " << classIdInt64 << "n;" << std::endl;
			}
			moduleFile << std::endl;
		}

		// Emit interfaces
		// Sort structs by inheritance hierarchy to avoid forward references
		if (module->serializableStructs.size() > 0)
		{
			std::vector<rfk::Struct const*> sortedStructs = topologicalSortStructsByInheritance(module->serializableStructs);

			for (rfk::Struct const* structRef : sortedStructs)
			{
				emitTypeScriptInterface(moduleFile, *structRef);
				moduleFile << std::endl;
			}
		}
	}

	void emitCSharpSerializableClass(std::ofstream& moduleFile, rfk::Struct const& structRef)
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
		const std::string& className= structRef.getName();
		moduleFile << "\tpublic class " << className << structInheritance << std::endl;
		moduleFile << "\t{" << std::endl;

		// Emit the refureku class id
		std::string classIdOverride = parentStructNames.size() > 0 ? "new " : "";
		size_t classId= structRef.getId();
		int64_t classIdInt64= *reinterpret_cast<int64_t*>(&classId);
		moduleFile << "\t\tpublic static " << classIdOverride << "readonly long classId= " << classIdInt64 << ";" << std::endl;
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

	void emitCSharpEnum(std::ofstream& moduleFile, rfk::Enum const& enumRef)
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
		else if (type == rfk::getType<std::string>())
		{
			return type.isCArray() ? "string[]" : "string";
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
			else if (cppType == "PolymorphicObjectPtr")
			{
				return "PolymorphicObject";
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				if (templateTypeName == "List" &&
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
			else if (type == rfk::getType<int32_t>() || type == rfk::getType<int>())
			{
				csType= "int";
			}
			else if (type == rfk::getType<uint64_t>())
			{
				csType= "ulong";
			}
			else if (type == rfk::getType<int64_t>() || type == rfk::getType<long>())
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

	// TypeScript Code Generation Methods
	// Topological sort to order structs so base classes come before derived classes
	std::vector<rfk::Struct const*> topologicalSortStructsByInheritance(const std::vector<rfk::Struct const*>& structs)
	{
		std::vector<rfk::Struct const*> result;
		std::set<rfk::Struct const*> visited;
		std::set<rfk::Struct const*> visiting;

		// Build a map of struct name to struct pointer for quick lookup
		std::map<std::string, rfk::Struct const*> nameToStruct;
		for (rfk::Struct const* s : structs)
		{
			nameToStruct[s->getName()] = s;
		}

		// Context struct to pass to the foreachDirectParent callback
		struct VisitContext {
			std::map<std::string, rfk::Struct const*>* nameToStruct;
			std::function<void(rfk::Struct const*)>* visit;
		};

		// Recursive DFS function
		std::function<void(rfk::Struct const*)> visit = [&](rfk::Struct const* s) {
			if (visited.count(s)) return;
			if (visiting.count(s)) return; // Cycle detected, skip

			visiting.insert(s);

			// Visit all parent structs first (only those in the same module)
			VisitContext ctx = { &nameToStruct, &visit };
			s->foreachDirectParent([](rfk::ParentStruct const& parentStruct, void* userData) -> bool {
				VisitContext* ctx = static_cast<VisitContext*>(userData);
				rfk::Struct const& parentArchetype = parentStruct.getArchetype();
				std::string parentName = parentArchetype.getName();

				// Check if parent is in our list of structs to sort
				auto it = ctx->nameToStruct->find(parentName);
				if (it != ctx->nameToStruct->end())
				{
					(*ctx->visit)(it->second);
				}

				return true;
			}, &ctx);

			visiting.erase(s);
			visited.insert(s);
			result.push_back(s);
		};

		// Visit all structs
		for (rfk::Struct const* s : structs)
		{
			visit(s);
		}

		return result;
	}

	void collectTypeScriptImports(ClientModulePtr const& module, std::set<std::string>& imports)
	{
		std::string currentModuleName = module->name;

		// Check parent types for structs
		for (rfk::Struct const* structRef : module->serializableStructs)
		{
			// Check parent structs
			auto parentParams = std::make_pair(&imports, &currentModuleName);
			structRef->foreachDirectParent([](rfk::ParentStruct const& parentStruct, void* userData) -> bool {
				auto* params = reinterpret_cast<std::pair<std::set<std::string>*, std::string*>*>(userData);
				std::set<std::string>* importsPtr = params->first;
				std::string* currentModulePtr = params->second;

				rfk::Struct const& parentArchetype = parentStruct.getArchetype();
				std::string parentName = parentArchetype.getName();

				// Get the module name of the parent
				std::string parentModuleName;
				Serialization::CodeGenModule const* property = parentArchetype.getProperty<Serialization::CodeGenModule>();
				if (property != nullptr)
				{
					parentModuleName = property->getModuleName();

					// Only add import if parent is from a different module
					if (parentModuleName != *currentModulePtr)
					{
						importsPtr->insert(parentName);
					}
				}
				else
				{
					// Parent doesn't have CodeGenModule property - it's probably a manual type like PolymorphicStruct
					// Always add it to imports so findModuleForType can handle it
					importsPtr->insert(parentName);
				}

				return true;
			}, &parentParams);

			// Check field types
			auto fieldParams = std::make_pair(&imports, &currentModuleName);
			structRef->foreachField([](rfk::Field const& field, void* userData) -> bool {
				auto* params = reinterpret_cast<std::pair<std::set<std::string>*, std::string*>*>(userData);
				std::set<std::string>* importsPtr = params->first;
				std::string* currentModulePtr = params->second;

				collectTypeScriptFieldImports(field, *importsPtr, *currentModulePtr);
				return true;
			}, &fieldParams);
		}
	}

	static void collectTypeScriptFieldImports(rfk::Field const& field, std::set<std::string>& imports, const std::string& currentModule)
	{
		rfk::Type const& fieldType = field.getType();
		collectTypeScriptTypeImports(fieldType, imports, currentModule);
	}

	static void collectTypeScriptTypeImports(rfk::Type const& type, std::set<std::string>& imports, const std::string& currentModule)
	{
		// Skip primitive types and built-in types
		if (type == rfk::getType<std::string>())
		{
			return;
		}

		rfk::Archetype const* archetype = type.getArchetype();
		if (!archetype) return;

		rfk::EEntityKind kind = archetype->getKind();
		std::string typeName = archetype->getName();

		// Skip primitive types and built-in types
		if (kind == rfk::EEntityKind::FundamentalArchetype ||
			typeName == "String" || 
			typeName == "BoolList")
		{
			return;
		}

		// Handle PolymorphicObjectPtr - needs both classes from parent module
		if (kind == rfk::EEntityKind::Class && typeName == "PolymorphicObjectPtr")
		{
			imports.insert("PolymorphicObject");
			imports.insert("PolymorphicStruct");
			return;
		}

		// Check if this type needs an import
		if (kind == rfk::EEntityKind::Struct || kind == rfk::EEntityKind::Enum)
		{
			// Get the module name of this type
			std::string typeModuleName;
			Serialization::CodeGenModule const* property = archetype->getProperty<Serialization::CodeGenModule>();
			if (property != nullptr)
			{
				typeModuleName = property->getModuleName();

				// Only add import if type is from a different module
				if (typeModuleName != currentModule)
				{
					imports.insert(typeName);
				}
			}
		}
		else if (kind == rfk::EEntityKind::Class)
		{
			// Handle template types (List<T>, Map<K,V>)
			rfk::Class const* classType = rfk::classCast(archetype);
			rfk::EClassKind classKind = classType->getClassKind();

			if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				// Recursively check template arguments (e.g., List<T>, Map<K,V>)
				// For List and Map templates, we know all arguments are type arguments
				if (templateTypeName == "List" || templateTypeName == "Map")
				{
					for (uint8_t i = 0; i < templateClassInstanceType->getTemplateArgumentsCount(); ++i)
					{
						auto const& templateArg =
							static_cast<rfk::TypeTemplateArgument const&>(
								templateClassInstanceType->getTemplateArgumentAt(i));
						collectTypeScriptTypeImports(templateArg.getType(), imports, currentModule);
					}
				}
			}
		}
	}

	void emitTypeScriptEnum(std::ofstream& moduleFile, rfk::Enum const& enumRef)
	{
		moduleFile << "export enum " << enumRef.getName() << " {" << std::endl;

		bool first = true;
		auto enumParams = std::make_pair(&moduleFile, &first);
		enumRef.foreachEnumValue([](rfk::EnumValue const& enumValue, void* userData) -> bool {
			auto* params = reinterpret_cast<std::pair<std::ofstream*, bool*>*>(userData);
			std::ofstream* moduleFilePtr = params->first;
			bool* firstPtr = params->second;

			auto const* property = enumValue.getProperty<Serialization::EnumStringValue>();
			if (property != nullptr)
			{
				if (!*firstPtr)
				{
					(*moduleFilePtr) << "," << std::endl;
				}
				*firstPtr = false;

				const std::string enumValueString = property->getValue();
				const int64_t enumInt64Value = enumValue.getValue();
				(*moduleFilePtr) << "  " << enumValueString << " = " << enumInt64Value;
			}

			return true;
		}, &enumParams);

		moduleFile << std::endl << "}" << std::endl;
	}

	static std::string getTypeScriptSerializationType(rfk::Type const& type)
	{
		rfk::Archetype const* archetype = type.getArchetype();
		rfk::EEntityKind fieldArchetypeKind = archetype ? archetype->getKind() : rfk::EEntityKind::Undefined;

		if (type.isPointer())
		{
			return "any";
		}
		else if (type == rfk::getType<std::string>())
		{
			return "string";
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Class)
		{
			rfk::Class const* classType = rfk::classCast(archetype);
			rfk::EClassKind classKind = classType->getClassKind();
			std::string cppType = classType->getName();

			if (cppType == "String")
			{
				return "string";
			}
			else if (cppType == "BoolList")
			{
				return "boolean";
			}
			else if (cppType == "PolymorphicObjectPtr")
			{
				return "PolymorphicObject";
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				if (templateTypeName == "List" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					auto const& templateArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& elementType = templateArg.getType();
					return getTypeScriptSerializationType(elementType);
				}
				else if (templateTypeName == "Map" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 2)
				{
					return "Map";
				}
			}
			else
			{
				return cppType;
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(archetype);
			return structType->getName();
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Enum)
		{
			rfk::Enum const* enumType = rfk::enumCast(archetype);
			return "enum:" + std::string(enumType->getName());
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::FundamentalArchetype)
		{
			if (type == rfk::getType<bool>())
			{
				return "boolean";
			}
			else if (type == rfk::getType<uint8_t>())
			{
				return "uint8";
			}
			else if (type == rfk::getType<int8_t>())
			{
				return "int8";
			}
			else if (type == rfk::getType<uint16_t>())
			{
				return "uint16";
			}
			else if (type == rfk::getType<int16_t>())
			{
				return "int16";
			}
			else if (type == rfk::getType<uint32_t>())
			{
				return "uint32";
			}
			else if (type == rfk::getType<int32_t>() || type == rfk::getType<int>())
			{
				return "int32";
			}
			else if (type == rfk::getType<uint64_t>())
			{
				return "uint64";
			}
			else if (type == rfk::getType<int64_t>() || type == rfk::getType<long>())
			{
				return "int64";
			}
			else if (type == rfk::getType<float>())
			{
				return "float";
			}
			else if (type == rfk::getType<double>())
			{
				return "double";
			}
		}

		return "any";
	}

	void emitTypeScriptInterface(std::ofstream& moduleFile, rfk::Struct const& structRef)
	{
		// Get parent classes
		using ParentList = std::vector<std::string>;
		ParentList parentStructNames;
		structRef.foreachDirectParent([](rfk::ParentStruct const& parentStruct, void* userData) -> bool {
			ParentList* parentStructNamesPtr = reinterpret_cast<ParentList*>(userData);
			std::string parentStructName = parentStruct.getArchetype().getName();
			parentStructNamesPtr->push_back(parentStructName);
			return true;
		}, &parentStructNames);

		// Generate class inheritance string
		std::string classInheritance;
		if (parentStructNames.size() > 0)
		{
			classInheritance = " extends ";
			for (size_t i = 0; i < parentStructNames.size(); ++i)
			{
				if (i > 0)
				{
					classInheritance += ", ";
				}
				classInheritance += parentStructNames[i];
			}
		}

		const std::string& className = structRef.getName();
		moduleFile << "export class " << className << classInheritance << " {" << std::endl;

		// Sort fields by memory offset
		using FieldList = std::vector<rfk::Field const*>;
		FieldList sortedFields;
		structRef.foreachField([](rfk::Field const& field, void* userData) -> bool {
			FieldList* sortedFieldsPtr = reinterpret_cast<FieldList*>(userData);
			sortedFieldsPtr->push_back(&field);
			return true;
		}, &sortedFields);

		std::sort(sortedFields.begin(), sortedFields.end(),
			[](rfk::Field const* a, rfk::Field const* b) {
			return a->getMemoryOffset() < b->getMemoryOffset();
		});

		// Emit fields with default initialization
		for (rfk::Field const* field : sortedFields)
		{
			std::string tsType = getTypeScriptType(*field);
			std::string tsDefaultValue = getTypeScriptDefaultValue(field->getType());
			moduleFile << "  " << field->getName() << ": " << tsType << " = " << tsDefaultValue << ";" << std::endl;
		}

		// Emit serialization metadata
		moduleFile << std::endl;
		moduleFile << "  static __serializationMetadata: Array<{name: string, type: string, isArray?: boolean, isMap?: boolean, keyType?: string, valueType?: string}> = [" << std::endl;

		for (size_t i = 0; i < sortedFields.size(); ++i)
		{
			rfk::Field const* field = sortedFields[i];
			rfk::Type const& fieldType = field->getType();
			std::string serializationType = getTypeScriptSerializationType(fieldType);

			moduleFile << "    { name: '" << field->getName() << "', type: '" << serializationType << "'";

			// Check if it's an array
			if (fieldType.getArchetype() && fieldType.getArchetype()->getKind() == rfk::EEntityKind::Class)
			{
				rfk::Class const* classType = rfk::classCast(fieldType.getArchetype());
				if (classType->getClassKind() == rfk::EClassKind::TemplateInstantiation)
				{
					const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
					std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

					if (templateTypeName == "List")
					{
						moduleFile << ", isArray: true";
					}
					else if (templateTypeName == "Map" && templateClassInstanceType->getTemplateArgumentsCount() == 2)
					{
						// Get key and value types
						auto const& keyArg = static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
						auto const& valueArg = static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(1));

						std::string keyType = getTypeScriptSerializationType(keyArg.getType());
						std::string valueType = getTypeScriptSerializationType(valueArg.getType());

						moduleFile << ", isMap: true, keyType: '" << keyType << "', valueType: '" << valueType << "'";
					}
				}
			}
			else if (fieldType.isCArray())
			{
				moduleFile << ", isArray: true";
			}

			moduleFile << " }";
			if (i < sortedFields.size() - 1)
			{
				moduleFile << ",";
			}
			moduleFile << std::endl;
		}

		moduleFile << "  ];" << std::endl;
		moduleFile << "}" << std::endl;
	}

	static std::string getTypeScriptDefaultValue(rfk::Type const& type)
	{
		rfk::Archetype const* archetype = type.getArchetype();
		rfk::EEntityKind kind = archetype ? archetype->getKind() : rfk::EEntityKind::Undefined;

		if (type.isPointer())
		{
			return "null";
		}
		else if (type == rfk::getType<std::string>())
		{
			return type.isCArray() ? "[]" : "''";
		}
		else if (kind == rfk::EEntityKind::Class)
		{
			rfk::Class const* classType = rfk::classCast(archetype);
			std::string cppType = classType->getName();

			if (cppType == "String")
			{
				return type.isCArray() ? "[]" : "''";
			}
			else if (cppType == "BoolList")
			{
				return "[]";
			}
			else if (cppType == "PolymorphicObjectPtr")
			{
				return "new PolymorphicObject()";
			}
			else if (classType->getClassKind() == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				if (templateTypeName == "List")
				{
					return "[]";
				}
				else if (templateTypeName == "Map")
				{
					return "{}";
				}
			}
			else
			{
				return "new " + cppType + "()";
			}
		}
		else if (kind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(archetype);
			std::string structTypeName = structType->getName();
			return type.isCArray() ? "[]" : "new " + structTypeName + "()";
		}
		else if (kind == rfk::EEntityKind::Enum)
		{
			if (type.isCArray())
			{
				return "[]";
			}
			else
			{
				// Get the first enum value as the default
				rfk::Enum const* enumType = rfk::enumCast(archetype);
				if (enumType && enumType->getEnumValuesCount() > 0)
				{
					std::string enumName = enumType->getName();

					const rfk::EnumValue& enumValue = enumType->getEnumValueAt(0);
					auto const* property = enumValue.getProperty<Serialization::EnumStringValue>();
					std::string firstValueName;
					if (property != nullptr)
					{
						firstValueName = property->getValue();
					}
					else
					{
						firstValueName = enumType->getEnumValueAt(0).getName();
					}

					return enumName + "." + firstValueName;

				}

				return "null as any";
			}
		}
		else if (kind == rfk::EEntityKind::FundamentalArchetype)
		{
			if (type == rfk::getType<bool>())
			{
				return type.isCArray() ? "[]" : "false";
			}
			else if (type == rfk::getType<uint64_t>() || type == rfk::getType<int64_t>())
			{
				return type.isCArray() ? "[]" : "0n";
			}
			else if (type == rfk::getType<uint8_t>() || type == rfk::getType<int8_t>() ||
					 type == rfk::getType<uint16_t>() || type == rfk::getType<int16_t>() ||
					 type == rfk::getType<uint32_t>() || type == rfk::getType<int32_t>() ||
					 type == rfk::getType<float>() || type == rfk::getType<double>())
			{
				return type.isCArray() ? "[]" : "0";
			}
		}

		return "null";
	}

	static std::string getTypeScriptType(rfk::Field const& field)
	{
		rfk::Type const& fieldType = field.getType();
		return getTypeScriptType(fieldType);
	}

	static std::string getTypeScriptType(rfk::Type const& type)
	{
		rfk::Archetype const* archetype = type.getArchetype();
		rfk::EEntityKind fieldArchetypeKind = archetype ? archetype->getKind() : rfk::EEntityKind::Undefined;

		if (type.isPointer())
		{
			return "any"; // Pointers become any in TypeScript
		}
		else if (type == rfk::getType<std::string>())
		{
			return type.isCArray() ? "string[]" : "string";
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
				return "boolean[]";
			}
			else if (cppType == "PolymorphicObjectPtr")
			{
				return "PolymorphicObject";
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(classType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				if (templateTypeName == "List" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					auto const& templateArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& elementType = templateArg.getType();
					std::string elementTypeString = getTypeScriptType(elementType);

					return elementTypeString + "[]";
				}
				else if (templateTypeName == "Map" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 2)
				{
					auto const& templateKeyArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& keyType = templateKeyArg.getType();
					std::string keyTypeString = getTypeScriptType(keyType);

					auto const& templateValueArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(1));
					rfk::Type const& valueType = templateValueArg.getType();
					std::string valueTypeString = getTypeScriptType(valueType);

					return "Record<" + keyTypeString + ", " + valueTypeString + ">";
				}
			}
			else
			{
				return type.isCArray() ? cppType + "[]" : cppType;
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(archetype);
			std::string structTypeName = structType->getName();

			return type.isCArray() ? structTypeName + "[]" : structTypeName;
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Enum)
		{
			rfk::Enum const* enumType = rfk::enumCast(archetype);
			std::string enumTypeName = enumType->getName();

			return type.isCArray() ? enumTypeName + "[]" : enumTypeName;
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::FundamentalArchetype)
		{
			std::string tsType;

			if (type == rfk::getType<bool>())
			{
				tsType = "boolean";
			}
			else if (type == rfk::getType<uint8_t>() ||
				type == rfk::getType<int8_t>() ||
				type == rfk::getType<uint16_t>() ||
				type == rfk::getType<int16_t>() ||
				type == rfk::getType<uint32_t>() ||
				type == rfk::getType<int32_t>() ||
				type == rfk::getType<float>() ||
				type == rfk::getType<double>())
			{
				tsType = "number";
			}
			else if (type == rfk::getType<uint64_t>() ||
				type == rfk::getType<int64_t>())
			{
				tsType = "bigint";
			}

			if (!tsType.empty())
			{
				return type.isCArray() ? tsType + "[]" : tsType;
			}
			else
			{
				tsType = "any";
			}
		}

		return "any";
	}

	static std::string toUpperSnakeCase(const std::string& str)
	{
		std::string result;
		for (size_t i = 0; i < str.length(); ++i)
		{
			char c = str[i];
			if (std::isupper(c) && i > 0 && std::islower(str[i - 1]))
			{
				result += '_';
			}
			result += std::toupper(c);
		}
		return result;
	}

	// Find which module a type belongs to
	std::string findModuleForType(const std::string& typeName)
	{
		// Special case: PolymorphicObject and PolymorphicStruct are imported from parent module
		if (typeName == "PolymorphicObject" || typeName == "PolymorphicStruct")
		{
			return "../PolymorphicObject";
		}

		if (!m_codeGenDatabase)
			return "";

		// Search through all modules to find which one contains this type
		for (const auto& modulePair : m_codeGenDatabase->modules)
		{
			const ClientModulePtr& module = modulePair.second;

			// Check enums
			for (const rfk::Enum* enumPtr : module->enums)
			{
				if (enumPtr->getName() == typeName)
				{
					return module->name;
				}
			}

			// Check structs
			for (const rfk::Struct* structPtr : module->serializableStructs)
			{
				if (structPtr->getName() == typeName)
				{
					return module->name;
				}
			}
		}

		return "";
	}

private:
	std::filesystem::path m_outputPath;
	TargetLanguage m_targetLanguage = TargetLanguage::CSharp;
	CodeGenDatabase* m_codeGenDatabase = nullptr;
};

//-- entry point -----
int main(int argc, char* argv[])
{
	IMikanAPIPtr apiPtr = IMikanAPI::createMikanAPI();
	MikanClientCodeGen app;

	return app.exec(argc, argv);
}