using System;
using System.Reflection;

namespace MikanXR
{
	public class SerializableObject<T> where T : class
	{
		private ulong _runtimeClassId= 0;
		public ulong RuntimeClassId => _runtimeClassId;

		private object _instance;
		public T Instance => _instance as T;

		public SerializableObject() 
		{ 
		}

		public SerializableObject(T instance)
		{
			setInstance(instance);
		}

		public void setInstance(T instance)
		{
			Type instanceType = instance.GetType();
			FieldInfo classIdField= 
				instanceType.GetField(
					"classId", 
					BindingFlags.Public | BindingFlags.Static);

			// All SerializableObject types have a static "classId" field
			// corresponding to the Refureku class ID in the C++ code
			_runtimeClassId = (ulong)classIdField.GetValue(null);
			_instance = instance;
		}
	}
}
