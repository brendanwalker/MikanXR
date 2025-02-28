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
		MikanClassId runtimeClassId;
		void* runtimeClassRawPtr;
	};

	PolymorphicObjectPtr::PolymorphicObjectPtr()
	{
		m_impl= new PolymorphicObjectPtrImpl;
		m_impl->serializableStructPtr = nullptr;
		m_impl->runtimeClassId = 0;
		m_impl->runtimeClassRawPtr = nullptr;
	}

	PolymorphicObjectPtr::PolymorphicObjectPtr(PolymorphicObjectPtr&& other) noexcept
		: m_impl(std::move(other.m_impl))
	{
		other.m_impl = nullptr;
	}

	PolymorphicObjectPtr::PolymorphicObjectPtr(const PolymorphicObjectPtr& other)
	{
		m_impl = new PolymorphicObjectPtrImpl;
		m_impl->serializableStructPtr = other.m_impl->serializableStructPtr;
		m_impl->runtimeClassId = other.m_impl->runtimeClassId;
		m_impl->runtimeClassRawPtr = other.m_impl->runtimeClassRawPtr;
	}

	PolymorphicObjectPtr& PolymorphicObjectPtr::operator=(PolymorphicObjectPtr&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			m_impl = std::move(other.m_impl);
			other.m_impl = nullptr;
		}

		return *this;
	}

	PolymorphicObjectPtr& PolymorphicObjectPtr::operator=(const PolymorphicObjectPtr& other)
	{
		if (this != &other)
		{
			reset();
			m_impl->serializableStructPtr = other.m_impl->serializableStructPtr;
			m_impl->runtimeClassId = other.m_impl->runtimeClassId;
			m_impl->runtimeClassRawPtr = other.m_impl->runtimeClassRawPtr;
		}

		return *this;
	}

	PolymorphicObjectPtr::~PolymorphicObjectPtr()
	{
		if (m_impl != nullptr)
		{
			delete m_impl;
			m_impl = nullptr;
		}
	}

	void PolymorphicObjectPtr::reset()
	{
		assert(m_impl != nullptr);
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