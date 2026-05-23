// picojson.h — Minimal picojson-compatible shim backed by nlohmann/json.
//
// Provides the picojson API surface required by LRDB (debugger.hpp,
// basic_server.hpp, message.hpp) without pulling in the real picojson library.
// nlohmann/json is used only for parse() and serialize(); all value storage
// uses native C++ types so get<object>() can return stable references.

#pragma once

#include <nlohmann/json.hpp>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace picojson {

	struct null {};

	class value;
	using object = std::map<std::string, value>;
	using array = std::vector<value>;

	// ---- value class -----------------------------------------------------------

	class value {
	public:
		enum type_t {
			NULL_TYPE,
			BOOL_TYPE,
			NUMBER_TYPE,
			STRING_TYPE,
			ARRAY_TYPE,
			OBJECT_TYPE
		};

		// Construction
		value() : type_(NULL_TYPE) {}
		explicit value(null) : type_(NULL_TYPE) {}
		explicit value(bool b) : type_(BOOL_TYPE), bool_(b) {}
		explicit value(double d) : type_(NUMBER_TYPE), number_(d) {}
		explicit value(const char* s) : type_(STRING_TYPE), string_(s) {}
		explicit value(const std::string& s) : type_(STRING_TYPE), string_(s) {}
		explicit value(std::string&& s)
			: type_(STRING_TYPE), string_(std::move(s)) {
		}
		explicit value(const array& a) : type_(ARRAY_TYPE), array_(a) {}
		explicit value(array&& a) : type_(ARRAY_TYPE), array_(std::move(a)) {}
		explicit value(const object& o) : type_(OBJECT_TYPE), object_(o) {}
		explicit value(object&& o) : type_(OBJECT_TYPE), object_(std::move(o)) {}

		// Type checking (specialised below)
		template <typename T>
		bool is() const;

		// Value extraction — returns references so callers can take const T& safely
		template <typename T>
		T& get();
		template <typename T>
		const T& get() const;

		// Object key access — returns null value if key not present (no throw)
		value get(const std::string& key) const {
			if (type_ == OBJECT_TYPE) {
				auto it = object_.find(key);
				if (it != object_.end()) return it->second;
			}
			return value();
		}
		bool contains(const std::string& key) const {
			return type_ == OBJECT_TYPE && object_.count(key) > 0;
		}

		bool evaluate_as_boolean() const {
			switch (type_) {
			case NULL_TYPE:   return false;
			case BOOL_TYPE:   return bool_;
			case NUMBER_TYPE: return number_ != 0;
			case STRING_TYPE: return !string_.empty();
			default:          return true;  // ARRAY_TYPE, OBJECT_TYPE
			}
		}

		// Serialise to JSON string
		std::string serialize() const { return to_nlohmann().dump(); }

		static value from_nlohmann(const nlohmann::json& j) {
			if (j.is_null()) return value();
			if (j.is_boolean()) return value(j.get<bool>());
			if (j.is_number()) return value(j.get<double>());
			if (j.is_string()) return value(j.get<std::string>());
			if (j.is_array()) {
				array a;
				a.reserve(j.size());
				for (const auto& elem : j) a.push_back(from_nlohmann(elem));
				return value(std::move(a));
			}
			if (j.is_object()) {
				object o;
				for (const auto& [k, v] : j.items()) o[k] = from_nlohmann(v);
				return value(std::move(o));
			}
			return value();
		}

	private:
		type_t type_;
		bool bool_ = false;
		double number_ = 0.0;
		std::string string_;
		array array_;
		object object_;

		nlohmann::json to_nlohmann() const {
			switch (type_) {
			case NULL_TYPE:
				return nullptr;
			case BOOL_TYPE:
				return bool_;
			case NUMBER_TYPE:
				return number_;
			case STRING_TYPE:
				return string_;
			case ARRAY_TYPE: {
				nlohmann::json a = nlohmann::json::array();
				for (const auto& v : array_) a.push_back(v.to_nlohmann());
				return a;
			}
			case OBJECT_TYPE: {
				nlohmann::json o = nlohmann::json::object();
				for (const auto& kv : object_) o[kv.first] = kv.second.to_nlohmann();
				return o;
			}
			}
			return nullptr;
		}

		friend std::string parse(value& out, const std::string& str);
	};

	// ---- is<T>() specialisations -----------------------------------------------

	template <>
	inline bool value::is<null>() const {
		return type_ == NULL_TYPE;
	}
	template <>
	inline bool value::is<bool>() const {
		return type_ == BOOL_TYPE;
	}
	template <>
	inline bool value::is<double>() const {
		return type_ == NUMBER_TYPE;
	}
	template <>
	inline bool value::is<std::string>() const {
		return type_ == STRING_TYPE;
	}
	template <>
	inline bool value::is<array>() const {
		return type_ == ARRAY_TYPE;
	}
	template <>
	inline bool value::is<object>() const {
		return type_ == OBJECT_TYPE;
	}

	// ---- get<T>() specialisations ----------------------------------------------

	template <>
	inline bool& value::get<bool>() {
		assert(type_ == BOOL_TYPE);
		return bool_;
	}
	template <>
	inline const bool& value::get<bool>() const {
		assert(type_ == BOOL_TYPE);
		return bool_;
	}

	template <>
	inline double& value::get<double>() {
		assert(type_ == NUMBER_TYPE);
		return number_;
	}
	template <>
	inline const double& value::get<double>() const {
		assert(type_ == NUMBER_TYPE);
		return number_;
	}

	template <>
	inline std::string& value::get<std::string>() {
		assert(type_ == STRING_TYPE);
		return string_;
	}
	template <>
	inline const std::string& value::get<std::string>() const {
		assert(type_ == STRING_TYPE);
		return string_;
	}

	template <>
	inline array& value::get<array>() {
		assert(type_ == ARRAY_TYPE);
		return array_;
	}
	template <>
	inline const array& value::get<array>() const {
		assert(type_ == ARRAY_TYPE);
		return array_;
	}

	template <>
	inline object& value::get<object>() {
		assert(type_ == OBJECT_TYPE);
		return object_;
	}
	template <>
	inline const object& value::get<object>() const {
		assert(type_ == OBJECT_TYPE);
		return object_;
	}

	// ---- parse() ---------------------------------------------------------------

	inline std::string parse(value& out, const std::string& str) {
		auto j = nlohmann::json::parse(str, nullptr, /*throw_on_error=*/false);
		if (j.is_discarded()) return "JSON parse error";
		out = value::from_nlohmann(j);
		return "";
	}

}  // namespace picojson
