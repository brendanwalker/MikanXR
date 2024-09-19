#include "JsonUtils.h"
#include "Logger.h"

#include <Refureku/Refureku.h>

namespace JsonUtils
{
	bool from_json(const json* jobject, void* instance, rfk::Struct const& archetype)
	{
		struct DeserializationArgs
		{
			const json* jobject;
			void* instance;
		};

		DeserializationArgs args = {jobject, instance};
		bool success = archetype.foreachField(
			[](rfk::Field const& field, void* userdata) -> bool {
				DeserializationArgs* args = reinterpret_cast<DeserializationArgs*>(userdata);
				const json* jobject= args->jobject;

				// Skip this field is it is const or is static
				if (!field.isMutable() || field.isStatic())
				{
					return true;
				}

				if (jobject->contains(field.getName()))
				{
					auto& jsonField = (*jobject)[field.getName()];

					return from_json(&jsonField, args->instance, field);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " not found in json";
					return false;
				}
			},
			&args,
			true);

		return success;
	}

	bool from_json(const json* jobject, void* instance, rfk::Field const& field)
	{
		// Error if this field is it is const or is static
		if (!field.isMutable())
		{
			MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
				<< "Field " << field.getName() << " was not mutable";
			return false;
		}
		if (field.isStatic())
		{
			MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
				<< "Field " << field.getName() << "is static";
			return false;
		}


		if (field.getKind() == rfk::EEntityKind::Enum)
		{
			// TODO
		}
		else if (field.getKind() == rfk::EEntityKind::Class || 
				field.getKind() == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(&field);

			if (jobject->is_object() && structType != nullptr)
			{
				void* fieldInstance = field.getPtrUnsafe(instance);

				return from_json(jobject, fieldInstance, *structType);
			}
			else
			{
				MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
					<< "Field " << field.getName() << " was not an object";
				return false;
			}
		}
		else if (field.getKind() == rfk::EEntityKind::Field)
		{
			if (field.getType() == rfk::getType<bool>())
			{
				if (jobject->is_boolean())
				{
					bool value = jobject->get<bool>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not a bool";
					return false;
				}
			}
			else if (field.getType() == rfk::getType<int>())
			{
				if (jobject->is_number_integer())
				{
					int value = jobject->get<int>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an int";
					return false;
				}
			}
			else if (field.getType() == rfk::getType<float>())
			{
				if (jobject->is_number_float())
				{
					float value = jobject->get<float>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not a float";
					return false;
				}
			}
			else if (field.getType() == rfk::getType<double>())
			{
				if (jobject->is_number_float())
				{
					double value = jobject->get<double>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not a double";
					return false;
				}
			}
			else if (field.getType() == rfk::getType<std::string>())
			{
				if (jobject->is_string())
				{
					std::string value = jobject->get<std::string>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not a string";
					return false;
				}
			}
		}
		else
		{
			MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
				<< "Field " << field.getName() << " has an unsupported type";
			return false;
		}

		return true;
	}
};