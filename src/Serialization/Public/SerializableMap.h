#pragma once

#include "SerializationExport.h"
#include "SerializableMap.rfkh.h"

#include <map>

namespace Serialization NAMESPACE()
{
	template <typename t_key, typename t_value>
	class CLASS() Map : public std::map<t_key, t_value>
	{
	public:
		METHOD()
		void clear() noexcept
		{
			return std::map<t_key, t_value>::clear();
		}

		METHOD()
		std::size_t size() const noexcept
		{
			return std::map<t_key, t_value>::size();
		}

		METHOD()
		const void* getOrAddRawValueMutable(const t_key& key)
		{
			auto it = std::map<t_key, t_value>::find(key);
			if (it != std::map<t_key, t_value>::end())
			{
				return &it->second;
			}
			else
			{
				auto result= std::map<t_key, t_value>::insert({key, t_value()});
				auto iter= result.first;

				return &iter->second;
			}
		}

		METHOD()
		const void* getRawValue(const t_key& key) const
		{
			auto it = std::map<t_key, t_value>::find(key);
			if (it != std::map<t_key, t_value>::end())
			{
				return &it->second;
			}
			else
			{
				return nullptr;
			}
		}

		METHOD()
		void* getRawValueMutable(const t_key& key)
		{
			return const_cast<void*>(getRawValue(key));
		}
		Serialization_Map_GENERATED
	};
};

File_SerializableMap_GENERATED