//-- includes -----
#include "SerializationProperty.h"
#include "Logger.h"

#include "Refureku/Refureku.h"

#include "stdio.h"

#include <string>
#include <map>
#include <memory>
#include <vector>


#ifdef _MSC_VER
#pragma warning(disable:4996)  // ignore strncpy warning
#endif

struct ClientModule
{
	std::string name;
	std::vector<rfk::Class const*> classes;
	std::vector<rfk::Struct const*> structs;
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
			module->classes.push_back(&entity);
		}
	}

	void visitStruct(rfk::Struct const& entity)
	{
		ClientModulePtr module = findOrAddModule(entity);

		if (module != nullptr)
		{
			module->structs.push_back(&entity);
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
		}
		else
		{
			MIKAN_LOG_ERROR("exec") << "Failed to initialize application!";
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


		return success;
	}

	void shutdown()
	{
		log_dispose();
	}

	void fetchModules(CodeGenDatabase& codeGenDatabase)
	{
		rfk::Database const& database= rfk::getDatabase();

		database.foreachFileLevelClass([](rfk::Class const& entity, void* userData) -> bool {
			auto* codeGenDB = reinterpret_cast<CodeGenDatabase*>(userData);
			codeGenDB->visitClass(entity);
			return true;
		}, &codeGenDatabase);

		database.foreachFileLevelClass([](rfk::Struct const& entity, void* userData) -> bool {
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
};

//-- entry point -----
int main(int argc, char* argv[])
{
	MikanClientCodeGen app;

	return app.exec(argc, argv);
}