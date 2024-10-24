#pragma once

#include "MikanScriptTypes.h"

#include "nlohmann/json.hpp"

inline void to_json(nlohmann::json& j, const MikanScriptMessageInfo& p)
{
	j = nlohmann::json{
		{"content", p.content.getValue()}
	};
}
inline void from_json(const nlohmann::json& j, MikanScriptMessageInfo& p)
{
	std::string content;
	from_json(j.at("content"), content);
	p.content.setValue(content);
}
