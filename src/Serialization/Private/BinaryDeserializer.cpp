#include "BinaryDeserializer.h"
#include "BinaryUtility.h"
#include "SerializationUtility.h"
#include "SerializableList.h"

#include "Refureku/Refureku.h"


namespace Serialization
{
	class BinaryReadVisitor : public IVisitor
	{
	public:
		BinaryReadVisitor(BinaryReader& binaryReader) : m_binaryReader(binaryReader) {}

		virtual void visitClass(ValueAccessor const& accessor) override
		{
			rfk::Type const& fieldType= accessor.getType();
			rfk::Class const* fieldClassType = accessor.getClassType();
			rfk::EClassKind classKind = fieldClassType->getClassKind();

			if (fieldType == rfk::getType<Serialization::BoolList>())
			{
				visitBoolList(accessor);
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
					visitList(accessor, *templateClassInstanceType);
				}
				else if (templateTypeName == "Map" &&
							templateClassInstanceType->getTemplateArgumentsCount() == 2)
				{
					visitMap(accessor, *templateClassInstanceType);
				}
			}
			else
			{
				throw std::runtime_error(
					stringify("BinaryWriteVisitor::visitClass() ",
								"Class Field ", accessor.getName(),
								" was not of expected type IEnumerable to deserialize json array value"));
			}
		}

		void visitBoolList(ValueAccessor const& arrayAccessor)
		{
			auto& boolList = arrayAccessor.getTypedValueMutableRef<Serialization::BoolList>();

			// Resize the array to the desired target size
			int32_t int32ArraySize = 0;
			from_binary(m_binaryReader, int32ArraySize);
			size_t arraySize = static_cast<size_t>(int32ArraySize);

			// Deserialize each element of the array
			boolList.resize(arraySize);
			for (size_t elementIndex = 0; elementIndex < arraySize; ++elementIndex)
			{
				bool value = false;
				from_binary(m_binaryReader, value);
				boolList[elementIndex] = value;
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
			rfk::Method const* resizeMethod = templatedArrayType.getMethodByName("resize");
			rfk::Method const* getRawElementMutableMethod = templatedArrayType.getMethodByName("getRawElementMutable");

			// Resize the array to the desired target size
			int32_t int32ArraySize= 0;
			from_binary(m_binaryReader, int32ArraySize);
			size_t arraySize = static_cast<size_t>(int32ArraySize);
			resizeMethod->invokeUnsafe<void>(arrayInstance, arraySize);

			// Deserialize each element of the array
			for (size_t elementIndex = 0; elementIndex < arraySize; ++elementIndex)
			{
				// Get the target element instance in the array
				void* elementInstance =
					getRawElementMutableMethod->invokeUnsafe<void*, const std::size_t&>(
						arrayInstance, elementIndex);

				// Make a fake "field" for an element in the array
				ValueAccessor elementAccessor(elementInstance, elementType);

				// Deserialize the element
				BinaryReadVisitor elementVisitor(m_binaryReader);
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

			// Get the number of pairs in the map
			int32_t pairCount = 0;
			from_binary(m_binaryReader, pairCount);

			// Deserialize each element of the array
			for (size_t pairIndex = 0; pairIndex < pairCount; ++pairIndex)
			{
				t_key key= t_key();
				from_binary(m_binaryReader, key);

				// Get or Add the target value instance in the map
				void* valueInstance =
					getOrAddValueMethod->invokeUnsafe<void*, const t_key&>(
						mapInstance, key);

				// Make a fake "field" for an element in the array
				ValueAccessor valueAccessor(valueInstance, valueType);

				// Deserialize the value
				BinaryReadVisitor valueVisitor(m_binaryReader);
				Serialization::visitValue(valueAccessor, &valueVisitor);
			}
		}

		virtual void visitStruct(ValueAccessor const& accessor) override
		{
			void* childObjectInstance = accessor.getUntypedValueMutablePtr();
			rfk::Struct const* structType = accessor.getStructType();
			BinaryReadVisitor jsonVisitor(m_binaryReader);

			Serialization::visitStruct(childObjectInstance, *structType, &jsonVisitor);
		}

		virtual void visitEnum(ValueAccessor const& accessor) override
		{
			rfk::Enum const& enumType = *accessor.getEnumType();
			rfk::Archetype const& enumArchetype = enumType.getUnderlyingArchetype();
			rfk::EnumValue const* enumValue = nullptr;

			std::string enumStringValue;
			from_binary(m_binaryReader, enumStringValue);
			enumValue = enumType.getEnumValueByName(enumStringValue.c_str());
			if (enumValue == nullptr)
			{
				throw std::runtime_error(
					stringify("BinaryWriteVisitor::visitEnum() ",
								"Enum Accessor ", accessor.getName(),
								" has an invalid value ", enumStringValue));
			}

			void* enumInstance = accessor.getInstanceMutable();
			rfk::Field const* enumField = accessor.getField();
			const int64_t enumInt64Value = enumValue->getValue();

			if (enumField != nullptr)
			{
				enumField->setUnsafe(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
			}
			else
			{
				std::memcpy(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
			}
		}

		virtual void visitBool(ValueAccessor const& accessor) override
		{
			deserializeValue<bool>(accessor);
		}

		virtual void visitByte(ValueAccessor const& accessor) override
		{
			deserializeValue<int8_t>(accessor);
		}

		virtual void visitUByte(ValueAccessor const& accessor) override
		{
			deserializeValue<uint8_t>(accessor);
		}

		virtual void visitShort(ValueAccessor const& accessor) override
		{
			deserializeValue<int16_t>(accessor);
		}

		virtual void visitUShort(ValueAccessor const& accessor) override
		{
			deserializeValue<uint16_t>(accessor);
		}

		virtual void visitInt(ValueAccessor const& accessor) override
		{
			deserializeValue<int32_t>(accessor);
		}

		virtual void visitUInt(ValueAccessor const& accessor) override
		{
			deserializeValue<uint32_t>(accessor);
		}

		virtual void visitLong(ValueAccessor const& accessor) override
		{
			deserializeValue<int64_t>(accessor);
		}

		virtual void visitULong(ValueAccessor const& accessor)
		{
			deserializeValue<uint64_t>(accessor);
		}

		virtual void visitFloat(ValueAccessor const& accessor) override
		{
			deserializeValue<float>(accessor);
		}

		virtual void visitDouble(ValueAccessor const& accessor) override
		{
			deserializeValue<double>(accessor);
		}

		virtual void visitString(ValueAccessor const& accessor) override
		{
			std::string value;
			std::string* variablePtr = accessor.getTypedValueMutablePtr<std::string>();

			from_binary(m_binaryReader, value);
			*variablePtr= value;
		}

	private:
		template <typename T>
		void deserializeValue(ValueAccessor const& accessor)
		{
			T value = T();
			from_binary(m_binaryReader, value);
			accessor.setValueByType(value);
		}

		BinaryReader& m_binaryReader;
	};

	// Public API
	bool deserializeFromJsonString(
		const std::vector<uint8_t>& inBytes,
		void* instance,
		rfk::Struct const& structType)
	{
		try
		{
			BinaryReader reader(inBytes.data(), inBytes.size());
			BinaryReadVisitor visitor(reader);
			Serialization::visitStruct(instance, structType, &visitor);

			return true;
		}
		catch (std::runtime_error* e)
		{
			return false;
		}
	}
};
