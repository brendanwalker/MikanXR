#include "JsonDeserializer.h"
#include "IEnumerator.h"
#include "SerializationUtility.h"
#include "SerializedList.h"

#include "nlohmann/json.hpp"
#include "Refureku/Refureku.h"

using json = nlohmann::json;

namespace Serialization
{
	class JsonReadVisitor : public IVisitor
	{
	public:
		JsonReadVisitor(const nlohmann::json& jsonObject) : m_jsonObject(jsonObject) {}

		virtual void visitClass(void* instance, rfk::Field const& field, rfk::Struct const& fieldClassType) override
		{
			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_array())
			{
				rfk::EClassKind classKind = fieldClassType.getClassKind();

				if (classKind == rfk::EClassKind::TemplateInstantiation)
				{
					auto const* templateInstanceType = rfk::classTemplateInstantiationCast(&fieldClassType);
					std::string templateTypeName = templateInstanceType ? templateInstanceType->getName() : "";

					if (templateTypeName == "List" &&
						templateInstanceType->getTemplateArgumentsCount() == 1)
					{
						auto const& templateArg= 
							static_cast<rfk::TypeTemplateArgument const& >(
								templateInstanceType->getTemplateArgumentAt(0));
						rfk::Type const& arrayElementType= templateArg.getType();
						void* arrayInstance = field.getPtrUnsafe(instance);

						visitList(arrayInstance, field, arrayElementType, fieldJsonObject);
					}
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
							"Class Field ", field.getName(),
							" was not of expected type IEnumerable to deserialize json array value"));
				}
			}
			else
			{
				JsonReadVisitor::visitStruct(instance, field, fieldClassType);
			}
		}

		void visitList(
			void* arrayInstance,
			rfk::Field const& arrayField,
			rfk::Type const& arrayElementType,
			const json& fieldJsonObject)
		{
			{
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
		}

		virtual void visitStruct(void* instance, rfk::Field const& field, rfk::Struct const& fieldStructType) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& childJsonObject = m_jsonObject[field.getName()];
			if (childJsonObject.is_object())
			{
				void* childObjectInstance = field.getPtrUnsafe(instance);

				JsonReadVisitor childVisitor(childJsonObject);
				Serialization::visitStruct(childObjectInstance, fieldStructType, &childVisitor);
			}
		}

		virtual void visitEnum(void* instance, rfk::Field const& field, rfk::Enum const& enumType) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_integer())
			{
				int value = fieldJsonObject.get<int>();
				rfk::EnumValue const* enumValue = enumType.getEnumValue(value);

				if (enumValue != nullptr)
				{
					field.setUnsafe(instance, enumValue);
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ", 
							"Enum Field ", field.getName(), 
							" has an invalid value ", value));
				}
			}
			else if (fieldJsonObject.is_string())
			{
				std::string value = fieldJsonObject.get<std::string>();
				rfk::EnumValue const* enumValue = enumType.getEnumValueByName(value.c_str());

				if (enumValue != nullptr)
				{
					int32_t enumIntValue = enumValue->getValue();
					field.setUnsafe(instance, enumIntValue);
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
									"Enum Field ", field.getName(),
									" has an invalid value ", value));
				}
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
								"Enum Field ", field.getName(),
								" was not an int or a string json value"));
			}
		}

		virtual void visitBool(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_boolean())
			{
				bool value = fieldJsonObject.get<bool>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Bool Field ", field.getName(),
							  " was not a bool json value"));
			}
		}

		virtual void visitByte(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_unsigned())
			{
				uint8_t value = fieldJsonObject.get<uint8_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Byte Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void VisitUByte(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_unsigned())
			{
				uint8_t value = fieldJsonObject.get<uint8_t>();

				field.setUnsafe(instance, value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint8_t value = (uint8_t)fieldJsonObject.get<int8_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UByte Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitShort(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_integer())
			{
				uint16_t value = fieldJsonObject.get<uint16_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Short Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitUShort(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_unsigned())
			{
				uint16_t value = fieldJsonObject.get<uint16_t>();

				field.setUnsafe(instance, value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint16_t value = (uint16_t)fieldJsonObject.get<int16_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UShort Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitInt(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_integer())
			{
				int32_t value = fieldJsonObject.get<int32_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Int32 Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitUInt(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_unsigned())
			{
				uint32_t value = fieldJsonObject.get<uint32_t>();

				field.setUnsafe(instance, value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint32_t value = (uint32_t)fieldJsonObject.get<int32_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UInt32 Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitLong(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_integer())
			{
				int64_t value = fieldJsonObject.get<int64_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Int64 Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitULong(void* instance, rfk::Field const& field)
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_unsigned())
			{
				uint64_t value = fieldJsonObject.get<uint64_t>();

				field.setUnsafe(instance, value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint64_t value = (uint64_t)fieldJsonObject.get<int64_t>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UInt64 Field ", field.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitFloat(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_float())
			{
				float value = fieldJsonObject.get<float>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Float Field ", field.getName(),
							  " was not a float json value"));
			}
		}

		virtual void visitDouble(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_number_float())
			{
				double value = fieldJsonObject.get<double>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Double Field ", field.getName(),
							  " was not a float json value"));
			}
		}

		virtual void visitString(void* instance, rfk::Field const& field) override
		{
			checkHasMatchingJsonFieldName(field);

			const json& fieldJsonObject = m_jsonObject[field.getName()];

			if (fieldJsonObject.is_string())
			{
				std::string value = fieldJsonObject.get<std::string>();

				field.setUnsafe(instance, value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "String Field ", field.getName(),
							  " was not a string json value"));
			}
		}

	private:
		void checkHasMatchingJsonFieldName(rfk::Field const& field) const
		{
			char const* fieldName = field.getName();

			if (!m_jsonObject.contains(fieldName))
			{
				throw std::runtime_error(stringify("Field ", field.getName(), " not found in json"));
			}
		}

		const nlohmann::json& m_jsonObject;
	};

	// Public API
	bool deserializefromJsonString(const std::string& jsonString, void* instance, rfk::Struct const& structType)
	{
		try
		{
			json jsonObject = json::parse(jsonString);
			JsonReadVisitor visitor(jsonObject);
			Serialization::visitStruct(instance, structType, &visitor);

			return true;
		}
		catch (json::parse_error* e)
		{
			return false;
		}
		catch (std::runtime_error* e)
		{
			return false;
		}
	}
};

