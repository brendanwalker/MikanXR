#pragma once

#include <string>
#include <sstream>

namespace Serialization
{
	// https://codereview.stackexchange.com/questions/46596/format-string-inline
	template <typename t_arg_type>
	std::string stringify(t_arg_type const& arg)
	{
		std::stringstream stringStream;
		stringStream << arg;
		return stringStream.str();
	}

	template<typename t_arg_type, typename... Args>
	std::string stringify(t_arg_type arg, const Args&... args)
	{
		return stringify(arg) + stringify(args...);
	}
}