using System;
using System.Diagnostics;
using System.Reflection;

namespace MikanXR
{
	internal class ValueAccessor
	{
		public ValueAccessor(object instance, Type type)
		{
			_instance = instance;
			_type = type;
			_name = type.Name;
		}

		public ValueAccessor(object instance, FieldInfo field)
		{
			_instance = instance;
			_fieldInfo = field;
			_type = field.FieldType;
			_name = field.Name;
		}

		public virtual T getValue<T>()
		{
			Debug.Assert(_type == typeof(T));
			Debug.Assert(_type.IsValueType);
			Debug.Assert(_instance != null);
			
			if (_fieldInfo != null)
			{
				return (T)_fieldInfo.GetValue(_instance);
			}
			else
			{
				return (T)_instance;
			}
		}

		public virtual void setValue<T>(T value)
		{
			Debug.Assert(_type == typeof(T));
			Debug.Assert(_type.IsValueType);
			Debug.Assert(_fieldInfo != null);

			if (_fieldInfo != null)
			{
				Debug.Assert(_instance != null);
				_fieldInfo.SetValue(_instance, value);
			}
			else
			{
				_instance= value;
			}
		}

		protected object _instance;
		public object ValueInstance => _instance;

		protected FieldInfo _fieldInfo;
		public FieldInfo ValueField => _fieldInfo;

		protected Type _type;
		public Type ValueType => _type;

		protected string _name;
		public string ValueName => _name;
	}

	internal interface IVisitor
	{
		void visitClass(ValueAccessor accessor);
		void visitList(ValueAccessor accessor);
		void visitDictionary(ValueAccessor accessor);
		void visitPolymorphicObject(ValueAccessor accessor);
		void visitEnum(ValueAccessor accessor);
		void visitBool(ValueAccessor accessor);
		void visitByte(ValueAccessor accessor);
		void visitUByte(ValueAccessor accessor);
		void visitShort(ValueAccessor accessor);
		void visitUShort(ValueAccessor accessor);
		void visitInt(ValueAccessor accessor);
		void visitUInt(ValueAccessor accessor);
		void visitLong(ValueAccessor accessor);
		void visitULong(ValueAccessor accessor);
		void visitFloat(ValueAccessor accessor);
		void visitDouble(ValueAccessor accessor);
		void visitString(ValueAccessor accessor);
	}

	internal static class Utils
	{
		public static object allocateMikanTypeByName(string typeName, out Type type)
		{
			object instance = null;
			type= Assembly.GetExecutingAssembly().GetType(typeName);

			if (type != null)
			{
				instance = Activator.CreateInstance(type);
			}

			return instance;
		}

		public static void visitObject<T>(T instance, IVisitor visitor) where T : struct
		{
			visitObject(instance, typeof(T), visitor);
		}

		public static void visitObject(object instance, Type structType, IVisitor visitor)
		{
			FieldInfo[] fields= structType.GetFields(BindingFlags.Instance | BindingFlags.Public);
			foreach (FieldInfo field in fields)
			{
				visitField(instance, field, visitor);
			}
		}

		public static void visitField(object instance, FieldInfo fieldInfo, IVisitor visitor)
		{
			visitValue(new ValueAccessor(instance, fieldInfo), visitor);
		}

		public static void visitValue(ValueAccessor accessor, IVisitor visitor)
		{
			Type type = accessor.ValueType;
			string name = accessor.ValueName;

			if (type.IsClass)
			{
				Type fieldType = accessor.ValueType;

				if (fieldType.IsGenericType)
				{
					if (fieldType.Name == "List" &&
						fieldType.GenericTypeArguments.Length == 1)
					{
						visitor.visitList(accessor);
					}
					else if (fieldType.Name == "Dictionary" &&
							fieldType.GenericTypeArguments.Length == 2)
					{
						visitor.visitDictionary(accessor);
					}
					else if (fieldType.Name == "SerializableObject" &&
							fieldType.GenericTypeArguments.Length == 1)
					{
						visitor.visitPolymorphicObject(accessor);
					}
					else
					{
						visitor.visitClass(accessor);
					}
				}
				else
				{
					visitor.visitClass(accessor);
				}
			}
			else if (type.IsEnum)
			{
				visitor.visitEnum(accessor);
			}
			else if (type.IsPrimitive)
			{
				switch (Type.GetTypeCode(type))
				{
					case TypeCode.Boolean:
						visitor.visitBool(accessor);
						break;
					case TypeCode.Byte:
						visitor.visitUByte(accessor);
						break;
					case TypeCode.SByte:
						visitor.visitByte(accessor);
						break;
					case TypeCode.Int16:
						visitor.visitShort(accessor);
						break;
					case TypeCode.UInt16:
						visitor.visitUShort(accessor);
						break;
					case TypeCode.Int32:
						visitor.visitInt(accessor);
						break;
					case TypeCode.UInt32:
						visitor.visitUInt(accessor);
						break;
					case TypeCode.Int64:
						visitor.visitLong(accessor);
						break;
					case TypeCode.UInt64:
						visitor.visitULong(accessor);
						break;
					case TypeCode.Single:
						visitor.visitFloat(accessor);
						break;
					case TypeCode.Double:
						visitor.visitDouble(accessor);
						break;
					case TypeCode.String:
						visitor.visitString(accessor);
						break;
					default:
						throw new NotImplementedException("Accessor " + name + " has unsupported type");
				}
			}
			else
			{
				throw new NotImplementedException("Accessor " + name + " has unsupported type");
			}
		}
	}
}
