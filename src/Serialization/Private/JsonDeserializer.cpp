#include "JsonDeserializer.h"
#include "SerializationUtility.h"
#include "SerializableList.h"
#include "SerializableMap.h"
#include "SerializableObjectPtr.h"
#include "SerializableString.h"

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

			rfk::Type const& fieldType = accessor.getType();
			rfk::Class const* fieldClassType = accessor.getClassType();
			rfk::EClassKind classKind = fieldClassType->getClassKind();

			if (fieldJsonObject.is_array())
			{
				if (fieldType == rfk::getType<Serialization::BoolList>())
				{
					visitBoolList(accessor, fieldJsonObject);
					return;
				}
				else if (classKind == rfk::EClassKind::TemplateInstantiation)
				{
					void* arrayInstance = accessor.getUntypedValueMutablePtr();
					const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(fieldClassType);
					std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

					// See if the field is a Serialization::List<T>
					if (templateTypeName == "List" &&
						templateClassInstanceType->getTemplateArgumentsCount() == 1)
					{
						visitList(accessor, *templateClassInstanceType, fieldJsonObject);
						return;
					}
					// See if the field is a Serialization::Map<K,V>
					else if (templateTypeName == "Map" &&
						templateClassInstanceType->getTemplateArgumentsCount() == 2)
					{
						visitMap(accessor, *templateClassInstanceType, fieldJsonObject);
						return;
					}
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonReadVisitor::visitClass() ",
							"Class Field ", accessor.getName(),
							" was not of expected type Serializable::List to deserialize json array value"));
				}
			}
			else if (fieldJsonObject.is_object())
			{
				if (classKind == rfk::EClassKind::TemplateInstantiation)
				{
					void* objectInstance = accessor.getUntypedValueMutablePtr();
					const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(fieldClassType);
					std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

					// See if the field is a Serialization::ObjectPtr<T>
					if (templateTypeName == "ObjectPtr" &&
						templateClassInstanceType->getTemplateArgumentsCount() == 1)
					{
						visitObjectPtr(accessor, *templateClassInstanceType, fieldJsonObject);
						return;
					}
				}
			}
			else if (fieldJsonObject.is_string())
			{
				if (fieldType == rfk::getType<Serialization::String>())
				{
					// A Serialization::String is a std::string
					std::string value = fieldJsonObject.get<std::string>();
					auto* variablePtr = accessor.getTypedValueMutablePtr<Serialization::String>();

					variablePtr->setValue(value);
					return;
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonReadVisitor::visitClass() ",
								  "Class Field ", accessor.getName(),
								  " was not of expected type Serializable::List to deserialize json array value"));
				}
			}

			JsonReadVisitor::visitStruct(accessor);
		}

		void visitObjectPtr(
			ValueAccessor const& sharedPtrAccessor,
			rfk::ClassTemplateInstantiation const& templatedArrayType,
			const json& ownerJsonObject)
		{
			// Get the shared pointer we are writing 
			void* sharedPtrInstance = sharedPtrAccessor.getUntypedValueMutablePtr();

			// Get the type of the elements in the array from the template argument
			auto const& templateArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedArrayType.getTemplateArgumentAt(0));
			rfk::Type const& elementType = templateArg.getType();
			rfk::Archetype const* elementArchetype = elementType.getArchetype();

			// Get the class for the object by type id
			std::size_t objectClassId = ownerJsonObject["class_id"].get<std::size_t>();
			rfk::Struct const* objectStruct = rfk::getDatabase().getStructById(objectClassId);
			if (objectStruct == nullptr)
			{
				throw std::runtime_error(
					stringify("JsonReadVisitor::visitObjectPtr() ",
							  "ObjectPtr Accessor ", sharedPtrAccessor.getName(),
							  " used an unknown class_id ", objectClassId,
							  ", archetype name: ", elementArchetype->getName()));
			}

			// Use reflection to get the methods to create and initialize the object
			rfk::Method const* allocateMethod = templatedArrayType.getMethodByName("allocate");

			// Allocate a default instance of the object assigned to the shared pointer
			void* objectInstance =
				allocateMethod->invokeUnsafe<void*, const std::size_t&>(
					sharedPtrInstance, objectClassId);

			// Deserialize the object from the json
			json objectJson= ownerJsonObject["value"];
			JsonReadVisitor objectVisitor(objectJson);
			Serialization::visitStruct(objectInstance, *objectStruct, &objectVisitor);
		}

		void visitBoolList(
			ValueAccessor const& arrayAccessor,
			const json& arrayJsonObject)
		{
			auto& boolListWrapper= arrayAccessor.getTypedValueMutableRef<Serialization::BoolList>();
			auto& boolList= boolListWrapper.getVectorMutable();
			std::size_t arraySize = arrayJsonObject.size();

			boolList.resize(arraySize);
			for (size_t elementIndex = 0; elementIndex < arraySize; ++elementIndex)
			{
				// Get the source json array element
				const json& elementJson = arrayJsonObject[elementIndex];

				if (elementJson.is_boolean())
				{
					bool value = elementJson.get<bool>();

					boolList[elementIndex] = value;
				}
				else
				{
					throw std::runtime_error(
						stringify("JsonReadVisitor::visitBool() ",
								  "Bool Accessor ", arrayAccessor.getName(),
								  "[", elementIndex, "] ",
								  " was not a bool json value"));
				}
			}
		}

		void visitList(
			ValueAccessor const& arrayAccessor,
			rfk::ClassTemplateInstantiation const& templatedArrayType,
			const json& arrayJsonObject)
		{
			void* arrayInstance= arrayAccessor.getUntypedValueMutablePtr();

			// Get the type of the elements in the array from the template argument
			auto const& templateArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedArrayType.getTemplateArgumentAt(0));
			rfk::Type const& elementType = templateArg.getType();

			// Use reflection to get the methods to resize the array and get a mutable reference to an element
			rfk::Method const* resizeMethod = templatedArrayType.getMethodByName("resize");
			rfk::Method const* getRawElementMutableMethod = templatedArrayType.getMethodByName("getRawElementMutable");

			// Resize the array to the desired target size
			std::size_t arraySize = arrayJsonObject.size();
			resizeMethod->invokeUnsafe<void>(arrayInstance, arraySize);

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

				// Deserialize the element
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
					stringify("JsonReadVisitor::visitMap() ",
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
			void* mapInstance = mapAccessor.getUntypedValueMutablePtr();

			// Get the type of the elements in the array from the template argument
			auto const& templateValueArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(1));
			rfk::Type const& valueType = templateValueArg.getType();

			// Use reflection to get the method use to clear and add pairs to the map
			rfk::Method const* clearMethod = templatedMapType.getMethodByName("clear");
			rfk::Method const* getOrAddValueMethod = templatedMapType.getMethodByName("getOrAddRawValueMutable");

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
						stringify("JsonReadVisitor::visitMapOfKey() ",
								  "Map Pair ", pairIndex,
								  " does not contain key"));
				}

				if (!pairJson.contains("value"))
				{
					throw std::runtime_error(
						stringify("JsonReadVisitor::visitMapOfKey() ",
								  "Map Pair ", pairIndex,
								  " does not contain value"));
				}

				// Parse the key from json
				t_key key= pairJson["key"].get<t_key>();

				// Get or Add the target value instance in the map
				void* valueInstance =
					getOrAddValueMethod->invokeUnsafe<void*, const t_key&>(
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
				void* childObjectInstance = accessor.getUntypedValueMutablePtr();
				rfk::Struct const* structType= accessor.getStructType();
				JsonReadVisitor jsonVisitor(childJsonObject);

				Serialization::visitStruct(childObjectInstance, *structType, &jsonVisitor);
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonReadVisitor::visitStruct() ",
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
						stringify("JsonReadVisitor::visitEnum() ",
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
						stringify("JsonReadVisitor::visitEnum() ",
									"Enum Accessor ", accessor.getName(),
									" has an invalid value ", value));
				}
			}
			else
			{
				throw std::runtime_error(
					stringify("JsonReadVisitor::visitEnum() ",
								"Enum Accessor ", accessor.getName(),
								" was not an int or a string json value"));
			}

			if (enumValue != nullptr)
			{
				void* enumInstance= accessor.getInstanceMutable();
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
					stringify("JsonReadVisitor::visitBool() ",
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
					stringify("JsonReadVisitor::visitByte() ",
							  "Byte Accessor ", accessor.getName(),
							  " was not a integer json value"));
			}
		}

		virtual void visitUByte(ValueAccessor const& accessor) override
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
					stringify("JsonReadVisitor::visitUByte() ",
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
					stringify("JsonReadVisitor::visitShort() ",
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
					stringify("JsonReadVisitor::visitUShort() ",
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
					stringify("JsonReadVisitor::visitInt() ",
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
					stringify("JsonReadVisitor::visitUInt() ",
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
					stringify("JsonReadVisitor::visitLong() ",
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
					stringify("JsonReadVisitor::visitULong() ",
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
					stringify("JsonReadVisitor::visitFloat() ",
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
					stringify("JsonReadVisitor::visitDouble() ",
							  "Double Accessor ", accessor.getName(),
							  " was not a float json value"));
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
	bool deserializeFromJsonString(const std::string& jsonString, void* instance, rfk::Struct const& structType)
	{
		try
		{
			json jsonObject = json::parse(jsonString);

			return deserializeFromJson(jsonObject, instance, structType);
		}
		catch (json::parse_error* e)
		{
			return false;
		}
	}

	bool deserializeFromJson(const nlohmann::json& jsonObject, void* instance, rfk::Struct const& structType)
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

