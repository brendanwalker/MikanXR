#include "BinarySerializer.h"
#include "BinaryUtility.h"
#include "SerializationUtility.h"
#include "SerializableList.h"
#include "SerializableMap.h"
#include "SerializableObjectPtr.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#include "Refureku/Refureku.h"

namespace Serialization
{
	class BinaryWriteVisitor : public IVisitor
	{
	public:
		BinaryWriteVisitor(BinaryWriter& writer) : m_binaryWriter(writer) {}

		virtual void visitClass(ValueAccessor const& accessor) override
		{
			rfk::Type const& fieldType= accessor.getType();
			rfk::Class const* fieldClassType = accessor.getClassType();
			rfk::EClassKind classKind = fieldClassType->getClassKind();

			if (fieldType == rfk::getType<Serialization::String>())
			{
				visitString(accessor);
			}
			else if (fieldType == rfk::getType<Serialization::BoolList>())
			{
				visitBoolList(accessor);
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(fieldClassType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				// See if the field is a Serialization::ObjectPtr<T>
				if (templateTypeName == "ObjectPtr" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					visitObjectPtr(accessor, *templateClassInstanceType);
				}
				// See if the field is a Serialization::List<T>
				else if (templateTypeName == "List" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					visitList(accessor, *templateClassInstanceType);
				}
				// See if the field is a Serialization::Map<K,V>
				else if (templateTypeName == "Map" &&
						 templateClassInstanceType->getTemplateArgumentsCount() == 2)
				{
					visitMap(accessor, *templateClassInstanceType);
				}
				else
				{
					throw std::runtime_error(
						stringify("BinaryWriteVisitor::visitClass() ",
								  "Class Field ", accessor.getName(),
								  " was of expected type"));

				}
			}
			else
			{
				BinaryWriteVisitor::visitStruct(accessor);
			}
		}

		void visitString(ValueAccessor const& accessor)
		{
			const auto* stringPtr= accessor.getTypedValuePtr<Serialization::String>();

			to_binary(m_binaryWriter, stringPtr->getValue());
		}

		void visitObjectPtr(
			ValueAccessor const& objectPtrAccessor,
			rfk::ClassTemplateInstantiation const& templatedArrayType)
		{
			// Get the shared pointer instance
			const void* sharedPtrInstance = objectPtrAccessor.getUntypedValuePtr();

			// Use reflection to get the runtime class id of the object pointed at
			rfk::Method const* getRuntimeClassIdMethod = templatedArrayType.getMethodByName("getRuntimeClassId");
			const std::size_t classId = getRuntimeClassIdMethod->invokeUnsafe<std::size_t>(sharedPtrInstance);

			// Get the runtime class for the object
			rfk::Struct const* objectStruct = rfk::getDatabase().getStructById(classId);
			if (objectStruct == nullptr)
			{
				throw std::runtime_error(
					stringify("BinaryWriteVisitor::visitObjectPtr() ",
							  "ObjectPtr Accessor ", objectPtrAccessor.getName(),
							  " has an invalid class id ", classId));
			}

			// Get the type of the elements in the array from the template argument
		#ifndef NDEBUG
			char const* objectStructName = objectStruct->getName();
		#endif

			// Write the runtime class id of the object
			to_binary(m_binaryWriter, (uint64_t)classId);

			// Get the raw pointer to the object pointed to by the shared pointer
			rfk::Method const* getRawPtrMethod = templatedArrayType.getMethodByName("getRawPtr");
			const void* objectInstance = getRawPtrMethod->invokeUnsafe<const void*>(sharedPtrInstance);

			// Write out whether the object is valid or not
			bool isValidObject = objectInstance != nullptr;
			to_binary(m_binaryWriter, isValidObject);

			// Serialize the object
			if (isValidObject)
			{
				BinaryWriteVisitor elementVisitor(m_binaryWriter);
				Serialization::visitStruct(objectInstance, *objectStruct, &elementVisitor);
			}
		}

		void visitBoolList(ValueAccessor const& arrayAccessor)
		{
			auto& boolListWrapper = arrayAccessor.getTypedValueRef<Serialization::BoolList>();
			auto& boolList = boolListWrapper.getVector();

			// Resize the array to the desired target size
			const size_t arraySize = boolList.size();
			const int32_t int32ArraySize = static_cast<int32_t>(arraySize);
			to_binary(m_binaryWriter, int32ArraySize);

			// Deserialize each element of the array
			for (size_t elementIndex = 0; elementIndex < arraySize; ++elementIndex)
			{
				const bool value = boolList[elementIndex];

				to_binary(m_binaryWriter, value);
			}
		}

		void visitList(
			ValueAccessor const& arrayAccessor,
			rfk::ClassTemplateInstantiation const& templatedArrayType)
		{
			void* arrayInstance = arrayAccessor.getUntypedValueMutablePtr();

			// Get the type of the elements in the array from the template argument
			auto const& templateArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedArrayType.getTemplateArgumentAt(0));
			rfk::Type const& elementType = templateArg.getType();

			// Use reflection to get the methods to resize the array and get a mutable reference to an element
			rfk::Method const* getSizeMethod = templatedArrayType.getMethodByName("size");
			rfk::Method const* getRawElementMethod = templatedArrayType.getMethodByName("getRawElement");

			// Write the size of the array
			int32_t arraySize = (int32_t)getSizeMethod->invokeUnsafe<std::size_t>(arrayInstance, arraySize);
			to_binary(m_binaryWriter, arraySize);

			// Serialize each element of the array
			for (size_t elementIndex = 0; elementIndex < arraySize; ++elementIndex)
			{
				// Get the target element instance in the array
				const void* elementInstance =
					getRawElementMethod->invokeUnsafe<const void*, const std::size_t&>(
						arrayInstance, elementIndex);

				// Make a fake "field" for an element in the array
				ValueAccessor elementAccessor(elementInstance, elementType);

				// Serialize the element into the json object
				BinaryWriteVisitor elementVisitor(m_binaryWriter);
				Serialization::visitValue(elementAccessor, &elementVisitor);
			}
		}

		void visitMap(
			ValueAccessor const& mapAccessor,
			rfk::ClassTemplateInstantiation const& templatedMapType)
		{
			// Get the key type of the map from the template argument
			auto const& templateKeyArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(0));
			rfk::Type const& keyType = templateKeyArg.getType();

			// Use reflection to get the the number of elements in the map
			rfk::Method const* getSizeMethod = templatedMapType.getMethodByName("size");

			// Write the number of elements in the map
			const void* mapInstance = mapAccessor.getUntypedValuePtr();
			int32_t arraySize = (int32_t)getSizeMethod->invokeUnsafe<std::size_t>(mapInstance);
			to_binary(m_binaryWriter, arraySize);

			// Do serialization based on the key type
			if (keyType == rfk::getType<int32_t>())
			{
				visitMapOfKey<int32_t>(mapAccessor, templatedMapType);
			}
			else if (keyType == rfk::getType<std::string>())
			{
				visitMapOfKey<std::string>(mapAccessor, templatedMapType);
			}
			else
			{
				rfk::Archetype const* keyArchetype = keyType.getArchetype();

				throw std::runtime_error(
					stringify("BinaryWriteVisitor::visitMap() ",
							  "Map Key Archetype ", keyArchetype != nullptr ? keyArchetype->getName() : "<Null Archetype>",
							  " is not supported"));
			}
		}

		template<typename t_key>
		void visitMapOfKey(
			ValueAccessor const& mapAccessor,
			rfk::ClassTemplateInstantiation const& templatedMapType)
		{
			// Get the type of the elements in the array from the template argument
			auto const& templateValueArg =
				static_cast<rfk::TypeTemplateArgument const&>(
					templatedMapType.getTemplateArgumentAt(1));
			rfk::Type const& valueType = templateValueArg.getType();

			// Use reflection to get the method used to enumerate key-value pairs in the map
			rfk::Method const* getConstEnumeratorMethod = templatedMapType.getMethodByName("getConstEnumerator");

			// Get a const enumerator to the map
			void* mapInstance = mapAccessor.getUntypedValueMutablePtr();

			// Serialize each element of the map
			for (auto enumerator =
				 getConstEnumeratorMethod->invokeUnsafe<std::shared_ptr<IMapConstEnumerator>>(mapInstance);
				 enumerator->isValid();
				 enumerator->next())
			{
				// Serialize the key
				const void* rawKey = enumerator->getKeyRaw();
				const t_key& key= *reinterpret_cast<const t_key*>(rawKey);
				to_binary(m_binaryWriter, key);

				// Serialize the value
				const void* rawValue = enumerator->getValueRaw();
				ValueAccessor valueAccessor(rawValue, valueType);
				BinaryWriteVisitor valueVisitor(m_binaryWriter);
				Serialization::visitValue(valueAccessor, &valueVisitor);
			}
		}

		virtual void visitStruct(ValueAccessor const& accessor) override
		{
			const void* childObjectInstance = accessor.getUntypedValuePtr();
			rfk::Struct const* structType = accessor.getStructType();
			rfk::Field const* field = accessor.getField();

			if (field != nullptr)
			{
				char const* fieldName = field->getName();
				BinaryWriteVisitor jsonVisitor(m_binaryWriter);
				Serialization::visitStruct(childObjectInstance, *structType, &jsonVisitor);
			}
			else
			{
				Serialization::visitStruct(childObjectInstance, *structType, this);
			}
		}

		virtual void visitEnum(ValueAccessor const& accessor) override
		{
			rfk::Enum const& enumType = *accessor.getEnumType();
			rfk::Archetype const& enumArchetype = enumType.getUnderlyingArchetype();
			const void* untypedValue = accessor.getUntypedValuePtr();

			int64_t enumIntValue = 0;
			if (enumArchetype.getMemorySize() == sizeof(int64_t))
			{
				enumIntValue = *reinterpret_cast<const int64_t*>(untypedValue);
			}
			else if (enumArchetype.getMemorySize() == sizeof(int32_t))
			{
				enumIntValue = (int64_t)(*reinterpret_cast<const int32_t*>(untypedValue));
			}
			else if (enumArchetype.getMemorySize() == sizeof(int16_t))
			{
				enumIntValue = (int64_t)(*reinterpret_cast<const int16_t*>(untypedValue));
			}
			else if (enumArchetype.getMemorySize() == sizeof(int8_t))
			{
				enumIntValue = (int64_t)(*reinterpret_cast<const int8_t*>(untypedValue));
			}
			else
			{
				throw std::runtime_error(
					stringify("BinaryWriteVisitor::visitEnum() ",
							  "Enum Accessor ", accessor.getName(),
							  " has an invalid memory size ", enumArchetype.getMemorySize()));
			}


			rfk::EnumValue const* enumValue = enumType.getEnumValue(enumIntValue);
			if (enumValue == nullptr)
			{
				throw std::runtime_error(
					stringify("BinaryWriteVisitor::visitEnum() ",
							  "Enum Accessor ", accessor.getName(),
							  " has an invalid int value ", enumIntValue));
			}


			std::string enumValueString = Serialization::getEnumStringValue(*enumValue);
			to_binary(m_binaryWriter, enumValueString);
		}

		virtual void visitBool(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<bool>());
		}

		virtual void visitByte(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<int8_t>());
		}

		virtual void visitUByte(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<uint8_t>());
		}

		virtual void visitShort(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<int16_t>());
		}

		virtual void visitUShort(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<uint16_t>());
		}

		virtual void visitInt(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<int32_t>());
		}

		virtual void visitUInt(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<uint32_t>());
		}

		virtual void visitLong(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<int64_t>());
		}

		virtual void visitULong(ValueAccessor const& accessor)
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<uint64_t>());
		}

		virtual void visitFloat(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<float>());
		}

		virtual void visitDouble(ValueAccessor const& accessor) override
		{
			to_binary(m_binaryWriter, *accessor.getTypedValuePtr<double>());
		}

	private:

		BinaryWriter& m_binaryWriter;
	};

	// Public API
	bool serializeToBytes(const void* instance, rfk::Struct const& structType, std::vector<uint8_t>& outBytes)
	{
		try
		{
			BinaryWriter writer(outBytes);
			BinaryWriteVisitor visitor(writer);
			Serialization::visitStruct(const_cast<void*>(instance), structType, &visitor);

			return true;
		}
		catch (std::runtime_error* e)
		{
			return false;
		}
	}
};

