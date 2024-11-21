using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MikanXR
{
	public class SerializableObject<T> where T : class
	{
		private Type _instanceRuntimeType;
		public string InstanceRuntimeTypeName => _instanceRuntimeType != null ? _instanceRuntimeType.Name : "";

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
			_instance = instance;
			_instanceRuntimeType = instance.GetType();
		}
	}
}
