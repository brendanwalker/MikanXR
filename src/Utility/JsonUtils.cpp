#include "JsonUtils.h"
#include "Logger.h"

#include <Refureku/Refureku.h>

namespace JsonUtils
{
	struct DeserializationArgs
	{
		const json* jobject;
		void* instance;
	};

	bool from_json(const json* jobject, void* instance, rfk::Struct const& archetype)
	{
		DeserializationArgs args = {jobject, instance};
		bool success = archetype.foreachField(
			[](rfk::Field const& field, void* userdata) -> bool {
				DeserializationArgs* args = reinterpret_cast<DeserializationArgs*>(userdata);
				const json* jobject= args->jobject;

				// Skip this field is it is non-public or is static
				if (field.getAccess() != rfk::EAccessSpecifier::Public || field.isStatic())
				{
					return true;
				}

				char const* fieldName = field.getName();
				if (jobject->contains(fieldName))
				{
					auto& jsonField = (*jobject)[fieldName];

					return from_json(&jsonField, args->instance, field);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << fieldName << " not found in json";
					return false;
				}
			},
			&args,
			true);

		return success;
	}

	bool from_json(const json* jobject, void* instance, rfk::Field const& field)
	{
		// Error if this field is non-public or is static
		if (field.getAccess() != rfk::EAccessSpecifier::Public)
		{
			MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
				<< "Field " << field.getName() << " was not public";
			return false;
		}
		if (field.isStatic())
		{
			MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
				<< "Field " << field.getName() << "is static";
			return false;
		}

		const rfk::EEntityKind fieldKind= field.getKind();
		rfk::Type const& fieldType = field.getType();
		rfk::Archetype const* fieldArchetype = fieldType.getArchetype();
		rfk::EEntityKind fieldArchetypeKind = fieldArchetype ? fieldArchetype->getKind() : rfk::EEntityKind::Undefined;
		const char* fieldArchetypeName = fieldArchetype ? fieldArchetype->getName() : "";

		if (fieldArchetypeKind == rfk::EEntityKind::Class)
		{
			rfk::Class const* classType = rfk::classCast(fieldArchetype);
			rfk::EClassKind classKind = classType->getClassKind();


			if (classKind == rfk::EClassKind::Template)
			{
				rfk::ClassTemplate const* templateDefinitionType = rfk::classTemplateCast(fieldArchetype);
				const char* templateTypeName = templateDefinitionType ? templateDefinitionType->getName() : "";

				MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
					<< "Field " << field.getName() << " was a pure template? Ignoring.";
				return true;
			}
			if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				//rfk::ClassTemplateInstantiation const* templateInstanceType = rfk::classTemplateInstantiationCast(fieldArchetype);
				//std::string templateTypeName = templateInstanceType ? templateInstanceType->getName() : "";

				//if (templateTypeName == "SerializedList" && 
				//	templateInstanceType->getTemplateArgumentsCount() == 1)
				//{
				//	templateInstanceType->getTemplateArgumentAt(0);

				//	if (jobject->is_array() && structType != nullptr)
				//	{
				//		const void* rawList = field.getPtrUnsafe(instance);
				//		size_t elementIndex = 0;

				//		rfk::Method const* getRawElementMethod = templateInstanceType->getMethodByName("getRawElement");
				//		const void* element= getRawElementMethod->invokeUnsafe<const void*, size_t>(rawList, elementIndex);


				//		return from_json(jobject, fieldInstance, *structType);
				//	}
				//	else
				//	{
				//		MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
				//			<< "Field " << field.getName() << " was not an object";
				//		return false;
				//	}
				//}
			}

			if (jobject->is_object() && classType != nullptr)
			{
				void* fieldInstance = field.getPtrUnsafe(instance);

				return from_json(jobject, fieldInstance, *classType);
			}
			else
			{
				MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
					<< "Field " << field.getName() << " was not an object";
				return false;
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(fieldArchetype);

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
		else if (fieldArchetypeKind == rfk::EEntityKind::Enum)
		{
			rfk::Enum const* enumType = rfk::enumCast(fieldArchetype);
			rfk::Archetype const& enumArchetype= enumType->getUnderlyingArchetype();

			if (enumType != nullptr)
			{
				if (jobject->is_number_integer())
				{
					int value = jobject->get<int>();
					rfk::EnumValue const* enumValue= enumType->getEnumValue(value);

					if (enumValue != nullptr)
					{
						field.setUnsafe(instance, enumValue);
					}
					else
					{
						MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
							<< "Field " << field.getName() << " has an invalid value";
						return false;
					}
				}
				else if (jobject->is_string())
				{
					std::string value = jobject->get<std::string>();
					rfk::EnumValue const* enumValue = enumType->getEnumValueByName(value.c_str());

					if (enumValue != nullptr)
					{
						int32_t enumIntValue= enumValue->getValue();
						field.setUnsafe(instance, enumIntValue);
					}
					else
					{
						MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
							<< "Field " << field.getName() << " has an invalid value";
						return false;
					}
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an int or a string";
					return false;
				}
			}
			else
			{
				MIKAN_MT_LOG_WARNING("JsonUtils::from_json()") 
					<< "Field " << field.getName() << " was not an enum";
				return false;
			}
		}
		else if (fieldKind == rfk::EEntityKind::Field)
		{			
			if (fieldType == rfk::getType<char>())
			{
				if (jobject->is_number_integer())
				{
					char value = (char)jobject->get<int>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not a char";
					return false;
				}
			}

			else if (fieldType == rfk::getType<bool>())
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
			else if (fieldType == rfk::getType<uint8_t>())
			{
				if (jobject->is_number_unsigned())
				{
					uint8_t value = jobject->get<uint8_t>();

					field.setUnsafe(instance, value);
				}
				else if (jobject->is_number_integer())
				{
					uint8_t value = (uint8_t)jobject->get<uint8_t>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an uint8_t";
					return false;
				}
			}
			else if (fieldType == rfk::getType<int8_t>())
			{
				if (jobject->is_number_integer())
				{
					int8_t value = jobject->get<int8_t>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an int8_t";
					return false;
				}
			}
			else if (fieldType == rfk::getType<uint16_t>())
			{
				if (jobject->is_number_unsigned())
				{
					uint16_t value = jobject->get<uint16_t>();

					field.setUnsafe(instance, value);
				}
				else if (jobject->is_number_integer())
				{
					uint16_t value = (uint16_t)jobject->get<uint16_t>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an uint16_t";
					return false;
				}
			}
			else if (fieldType == rfk::getType<int16_t>())
			{
				if (jobject->is_number_integer())
				{
					int16_t value = jobject->get<int16_t>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an int16_t";
					return false;
				}
			}
			else if (fieldType == rfk::getType<uint32_t>())
			{
				if (jobject->is_number_unsigned())
				{
					uint32_t value = jobject->get<uint32_t>();

					field.setUnsafe(instance, value);
				}
				else if (jobject->is_number_integer())
				{
					uint32_t value = (uint32_t)jobject->get<int>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an uint32_t";
					return false;
				}
			}
			else if (fieldType == rfk::getType<int64_t>())
			{
				if (jobject->is_number_integer())
				{
					int64_t value = jobject->get<int64_t>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an int64_t";
					return false;
				}
				}
			else if (fieldType == rfk::getType<uint64_t>())
			{
				if (jobject->is_number_unsigned())
				{
					uint64_t value = jobject->get<uint64_t>();

					field.setUnsafe(instance, value);
				}
				else if (jobject->is_number_integer())
				{
					uint64_t value = (uint64_t)jobject->get<int64_t>();

					field.setUnsafe(instance, value);
				}
				else
				{
					MIKAN_MT_LOG_WARNING("JsonUtils::from_json()")
						<< "Field " << field.getName() << " was not an uint32_t";
					return false;
				}
			}
			else if (fieldType == rfk::getType<int>())
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
			else if (fieldType == rfk::getType<float>())
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
			else if (fieldType == rfk::getType<double>())
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
			else if (fieldType == rfk::getType<std::string>())
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