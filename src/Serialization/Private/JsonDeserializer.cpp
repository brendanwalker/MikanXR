#include "JsonDeserializer.h"
#include "SerializationUtility.h"
#include "SerializableList.h"

#include "nlohmann/json.hpp"
#include "Refureku/Refureku.h"

using json = nlohmann::json;

namespace Serialization
{
	class JsonReadVisitor : public IVisitor
	{
	public:
		JsonReadVisitor(const nlohmann::json& jsonObject) : m_jsonObject(jsonObject) {}

		virtual void visitClass(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject = getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_array())
			{
				rfk::Class const* fieldClassType= accessor.getClassType();
				rfk::EClassKind classKind = fieldClassType->getClassKind();

				if (classKind == rfk::EClassKind::TemplateInstantiation)
				{
					void* arrayInstance = accessor.getUntypedValuePtr();
					const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(fieldClassType);
					std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

					// See if the field is a Serialization::List<T>
					if (templateTypeName == "List" &&
						templateClassInstanceType->getTemplateArgumentsCount() == 1)
					{
						visitList(accessor, *templateClassInstanceType, fieldJsonObject);
					}
					else if (templateTypeName == "Map" &&
						templateClassInstanceType->getTemplateArgumentsCount() == 2)
					{
						visitMap(accessor, *templateClassInstanceType, fieldJsonObject);
					}
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
							"Class Field ", accessor.getName(),
							" was not of expected type IEnumerable to deserialize json array value"));
				}
			}
			else
			{
				JsonReadVisitor::visitStruct(accessor);
			}
		}

		void visitList(
			ValueAccessor const& arrayAccessor,
			rfk::ClassTemplateInstantiation const& templatedArrayType,
			const json& arrayJsonObject)
		{
			void* arrayInstance= arrayAccessor.getUntypedValuePtr();

			// Get the type of the elements in the array from the template argument
			auto const& templateArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedArrayType.getTemplateArgumentAt(0));
			rfk::Type const& elementType = templateArg.getType();

			// Use reflection to get the methods to resize the array and get a mutable reference to an element
			rfk::Method const* getResizeMethod = templatedArrayType.getMethodByName("resize");
			rfk::Method const* getRawElementMutableMethod = templatedArrayType.getMethodByName("getRawElementMutable");

			// Resize the array to the desired target size
			std::size_t arraySize = arrayJsonObject.size();
			getResizeMethod->invokeUnsafe<void>(arrayInstance, arraySize);

			// Deserialize each element of the array
			for (size_t elementIndex= 0; elementIndex < arraySize; ++elementIndex) 
			{
				// Get the source json array element
				const json& elementJson = arrayJsonObject[elementIndex];

				// Get the target element instance in the array
				void* elementInstance= 
					getRawElementMutableMethod->invokeUnsafe<void*, const std::size_t&>(
						arrayInstance, elementIndex);

				// Make a fake "field" for an element in the array
				ValueAccessor elementAccessor(elementInstance, elementType);

				// Deserialize the element into the 
				JsonReadVisitor elementVisitor(elementJson);
				Serialization::visitValue(elementAccessor, &elementVisitor);
			}
		}

		void visitMap(
			ValueAccessor const& mapAccessor,
			rfk::ClassTemplateInstantiation const& templatedMapType,
			const json& mapArrayJsonObject)
		{
			// Get the key type of the map from the template argument
			auto const& templateKeyArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(0));
			rfk::Type const& keyType = templateKeyArg.getType();

			if (keyType == rfk::getType<int32_t>())
			{
				visitMapOfKey<int32_t>(mapAccessor, templatedMapType, mapArrayJsonObject);
			}
			else if (keyType == rfk::getType<std::string>())
			{
				visitMapOfKey<std::string>(mapAccessor, templatedMapType, mapArrayJsonObject);
			}
			else
			{
				rfk::Archetype const* keyArchetype= keyType.getArchetype();

				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
						"Map Key Archetype ", keyArchetype != nullptr ? keyArchetype->getName() : "<Null Archetype>",
						" is not supported"));
			}
		}

		template<typename t_key>
		void visitMapOfKey(
			ValueAccessor const& mapAccessor,
			rfk::ClassTemplateInstantiation const& templatedMapType,
			const json& mapArrayJsonObject)
		{
			void* mapInstance = mapAccessor.getUntypedValuePtr();

			// Get the type of the elements in the array from the template argument
			auto const& templateValueArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(1));
			rfk::Type const& valueType = templateValueArg.getType();

			// Use reflection to get the method use to clear and add pairs to the map
			rfk::Method const* clearMethod = templatedMapType.getMethodByName("clear");
			rfk::Method const* addValueMethod = templatedMapType.getMethodByName("getOrAddRawValueMutable");

			// Resize the array to the desired target size
			clearMethod->invokeUnsafe<void>(mapInstance);

			// Deserialize each element of the array
			std::size_t pairCount = mapArrayJsonObject.size();
			for (size_t pairIndex = 0; pairIndex < pairCount; ++pairIndex)
			{
				// Get the source json array element
				const json& pairJson = mapArrayJsonObject[pairIndex];

				if (!pairJson.contains("key"))
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
								  "Map Pair ", pairIndex,
								  " does not contain key"));
				}

				if (!pairJson.contains("value"))
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
								  "Map Pair ", pairIndex,
								  " does not contain value"));
				}

				// Parse the key from json
				t_key key= pairJson["key"].get<t_key>();

				// Get or Add the target value instance in the map
				void* valueInstance =
					addValueMethod->invokeUnsafe<void*, const t_key&>(
						mapInstance, key);

				// Make a fake "field" for an element in the array
				ValueAccessor valueAccessor(valueInstance, valueType);

				// Deserialize the value
				JsonReadVisitor valueVisitor(pairJson["value"]);
				Serialization::visitValue(valueAccessor, &valueVisitor);
			}
		}

		virtual void visitStruct(ValueAccessor const& accessor) override
		{
			const json& childJsonObject= getJsonObjectFromAccessor(accessor);

			if (childJsonObject.is_object())
			{
				void* childObjectInstance = accessor.getUntypedValuePtr();
				rfk::Struct const* structType= accessor.getStructType();
				JsonReadVisitor jsonVisitor(childJsonObject);

				Serialization::visitStruct(childObjectInstance, *structType, &jsonVisitor);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
						"Struct Field ", accessor.getName(),
						" was not of expected type object to deserialize json object value"));
			}
		}

		virtual void visitEnum(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);
			rfk::Enum const& enumType = *accessor.getEnumType();
			rfk::Archetype const& enumArchetype = enumType.getUnderlyingArchetype();
			rfk::EnumValue const* enumValue = nullptr;

			if (fieldJsonObject.is_number_integer())
			{
				int value = fieldJsonObject.get<int>();
				enumValue = enumType.getEnumValue(value);

				if (enumValue == nullptr)
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
								  "Enum Accessor ", accessor.getName(),
								  " has an invalid value ", value));
				}
			}
			else if (fieldJsonObject.is_string())
			{
				std::string value = fieldJsonObject.get<std::string>();
				enumValue = enumType.getEnumValueByName(value.c_str());

				if (enumValue == nullptr)
				{
					throw std::runtime_error(
						stringify("JsonUtils::from_json() ",
									"Enum Accessor ", accessor.getName(),
									" has an invalid value ", value));
				}
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
								"Enum Accessor ", accessor.getName(),
								" was not an int or a string json value"));
			}

			if (enumValue != nullptr)
			{
				void* enumInstance= accessor.getInstance();
				rfk::Field const* enumField= accessor.getField();
				const int64_t enumInt64Value= enumValue->getValue();

				if (enumField != nullptr)
				{
					enumField->setUnsafe(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
				}
				else
				{
					std::memcpy(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
				}
			}
		}

		virtual void visitBool(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_boolean())
			{
				bool value = fieldJsonObject.get<bool>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Bool Accessor ", accessor.getName(),
							  " was not a bool json value"));
			}
		}

		virtual void visitByte(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number())
			{
				int8_t value = fieldJsonObject.get<int8_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Byte Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void VisitUByte(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_unsigned())
			{
				uint8_t value = fieldJsonObject.get<uint8_t>();

				accessor.setValueByType(value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint8_t value = (uint8_t)fieldJsonObject.get<int8_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UByte Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitShort(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_integer())
			{
				int16_t value = fieldJsonObject.get<int16_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Short Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitUShort(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_unsigned())
			{
				uint16_t value = fieldJsonObject.get<uint16_t>();

				accessor.setValueByType(value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint16_t value = (uint16_t)fieldJsonObject.get<int16_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UShort Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitInt(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_integer())
			{
				int32_t value = fieldJsonObject.get<int32_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Int32 Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitUInt(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_unsigned())
			{
				uint32_t value = fieldJsonObject.get<uint32_t>();

				accessor.setValueByType(value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint32_t value = (uint32_t)fieldJsonObject.get<int32_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UInt32 Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitLong(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_integer())
			{
				int64_t value = fieldJsonObject.get<int64_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Int64 Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitULong(ValueAccessor const& accessor)
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_unsigned())
			{
				uint64_t value = fieldJsonObject.get<uint64_t>();

				accessor.setValueByType(value);
			}
			else if (fieldJsonObject.is_number_integer())
			{
				uint64_t value = (uint64_t)fieldJsonObject.get<int64_t>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "UInt64 Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitFloat(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_float())
			{
				float value = fieldJsonObject.get<float>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Float Accessor ", accessor.getName(),
							  " was not a float json value"));
			}
		}

		virtual void visitDouble(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_number_float())
			{
				double value = fieldJsonObject.get<double>();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "Double Accessor ", accessor.getName(),
							  " was not a float json value"));
			}
		}

		virtual void visitString(ValueAccessor const& accessor) override
		{
			const json& fieldJsonObject= getJsonObjectFromAccessor(accessor);

			if (fieldJsonObject.is_string())
			{
				std::string value = fieldJsonObject.get<std::string>();
				std::string *variablePtr= accessor.getTypedValuePtr<std::string>();

				*variablePtr= value;
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonUtils::from_json() ",
							  "String Accessor ", accessor.getName(),
							  " was not a string json value"));
			}
		}

	private:
		const json& getJsonObjectFromAccessor(ValueAccessor const& accessor) const
		{
			rfk::Field const* field = accessor.getField();

			if (field != nullptr)
			{
				char const* fieldName = field->getName();

				if (!m_jsonObject.contains(fieldName))
				{
					throw std::runtime_error(stringify("Field ", field->getName(), " not found in json"));
				}

				return m_jsonObject[field->getName()];
			}
			else
			{
				// The json object should contain the value we want to deserialize
				return m_jsonObject;
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

			return deserializefromJson(jsonObject, instance, structType);
		}
		catch (json::parse_error* e)
		{
			return false;
		}
	}

	bool deserializefromJson(const nlohmann::json& jsonObject, void* instance, rfk::Struct const& structType)
	{
		try
		{
			JsonReadVisitor visitor(jsonObject);
			Serialization::visitStruct(instance, structType, &visitor);

			return true;
		}
		catch (std::runtime_error* e)
		{
			return false;
		}
	}
};

