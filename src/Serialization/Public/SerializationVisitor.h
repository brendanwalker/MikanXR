#pragma once

namespace rfk
{
	class Enum;
	class Field;
	class Struct;
	using Class = Struct;
};

namespace Serialization
{
	class IVisitor
	{
	public:
		virtual void visitClass(void* instance, rfk::Field const& field, rfk::Struct const& fieldClassType) { }
		virtual void visitStruct(void* instance, rfk::Field const& field, rfk::Struct const& fieldStructType) { }
		virtual void visitEnum(void* instance, rfk::Field const& field, rfk::Enum const& fieldEnumType) { }
		virtual void visitBool(void* instance, rfk::Field const& field) { }
		virtual void visitByte(void* instance, rfk::Field const& field) { }
		virtual void VisitUByte(void* instance, rfk::Field const& field) { }
		virtual void visitShort(void* instance, rfk::Field const& field) { }
		virtual void visitUShort(void* instance, rfk::Field const& field) { }
		virtual void visitInt(void* instance, rfk::Field const& field) { }
		virtual void visitUInt(void* instance, rfk::Field const& field) { }
		virtual void visitLong(void* instance, rfk::Field const& field) { }
		virtual void visitULong(void* instance, rfk::Field const& field) { }
		virtual void visitFloat(void* instance, rfk::Field const& field) { }
		virtual void visitDouble(void* instance, rfk::Field const& field) { }
		virtual void visitString(void* instance, rfk::Field const& field) { }
	};

	template <typename t_struct_type>
	void visitStruct(t_struct_type& instance, IVisitor *visitor)
	{
		visitStruct(&instance, t_struct_type::staticGetArchetype(), visitor, userdata);
	}

	void visitStruct(void* instance, rfk::Struct const& structType, IVisitor *visitor);
	void visitField(void* instance, rfk::Field const& fieldType, IVisitor *visitor);
};