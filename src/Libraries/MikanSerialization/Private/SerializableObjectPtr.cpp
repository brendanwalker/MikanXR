#include "SerializableObjectPtr.h"
#include "SerializableObjectPtr.rfks.h"

namespace Serialization
{
	MikanClassId toMikanClassId(RfkClassId classId)
	{
		return *reinterpret_cast<MikanClassId*>(&classId);
	}

	RfkClassId toRfkClassId(MikanClassId classId)
	{
		return *reinterpret_cast<RfkClassId*>(&classId);
	}

	struct PolymorphicObjectPtrImpl
	{
		std::shared_ptr<PolymorphicStruct> serializableStructPtr;
		MikanClassId runtimeClassId = 0;
		void* runtimeClassRawPtr= nullptr;
	};

	PolymorphicObjectPtr::PolymorphicObjectPtr() : m_impl(new PolymorphicObjectPtrImpl())
	{
	}

	PolymorphicObjectPtr::~PolymorphicObjectPtr()
	{
		freeObject();
		delete m_impl;
	}

	void PolymorphicObjectPtr::freeObject()
	{
		m_impl->serializableStructPtr.reset();
		m_impl->runtimeClassId= 0;
		m_impl->runtimeClassRawPtr = nullptr;
	}

	void* PolymorphicObjectPtr::allocateByClassId(const std::size_t&& rfkClassId)
	{
		rfk::Struct const* objectClass = rfk::getDatabase().getStructById(rfkClassId);
		if (objectClass != nullptr)
		{
			auto objectPtr= objectClass->makeSharedInstance<PolymorphicStruct>();

			setPolymorphicStructPtrInternal(
				objectPtr,
				*objectClass);

			return getRawPtrMutable();
		}

		return nullptr;
	}

	const void* PolymorphicObjectPtr::getRawPtr() const
	{
		return m_impl->runtimeClassRawPtr;
	}

	void* PolymorphicObjectPtr::getRawPtrMutable()
	{
		return const_cast<void *>(getRawPtr());
	}

	std::size_t PolymorphicObjectPtr::getRuntimeClassId() const
	{
		return m_impl->runtimeClassId;
	}

	void PolymorphicObjectPtr::setPolymorphicStructPtrInternal(
		std::shared_ptr<PolymorphicStruct> objPtr,
		rfk::Struct const& objectClass)
	{
		uint8_t* rawObjPtr = reinterpret_cast<uint8_t*>(objPtr.get());

		std::ptrdiff_t pointerOffset;
		if (!PolymorphicStruct::staticGetArchetype().getSubclassPointerOffset(objectClass, pointerOffset))
		{
			pointerOffset= 0;
		}

		m_impl->serializableStructPtr = objPtr;
		m_impl->runtimeClassId = toMikanClassId(objectClass.getId());
		m_impl->runtimeClassRawPtr = rawObjPtr - pointerOffset;
	}
}