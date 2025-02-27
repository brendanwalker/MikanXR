using System;
using System.Reflection;

namespace MikanXR
{
	public class PolymorphicStruct 
	{
	}

	public class PolymorphicObject
	{
		private long _runtimeClassId= 0;
		public long RuntimeClassId => _runtimeClassId;

		private PolymorphicStruct _instance;
		public PolymorphicStruct Instance => _instance;

		public PolymorphicObject() 
		{ 
		}

		public PolymorphicObject(PolymorphicStruct instance)
		{
			setInstance(instance);
		}

		public void setInstance(PolymorphicStruct instance)
		{
			Type instanceType = instance.GetType();
			FieldInfo classIdField= 
				instanceType.GetField(
					"classId", 
					BindingFlags.Public | BindingFlags.Static);

			// All SerializableObject types have a static "classId" field
			// corresponding to the Refureku class ID in the C++ code
			_runtimeClassId = (long)classIdField.GetValue(null);
			_instance = instance;
		}
	}
}
