#include "literal.hpp"

namespace components::types {

literal_t::literal_t() : type_(literal_type::null), value_(std::monostate{}) {}
literal_t::literal_t(int64_t v) : type_(literal_type::integer), value_(v) {}
literal_t::literal_t(bool v) : type_(literal_type::boolean), value_(v) {}
literal_t::literal_t(const std::string& v) : type_(literal_type::string), value_(v) {}
literal_t::literal_t(std::string&& v) : type_(literal_type::string), value_(std::move(v)) {}

literal_type literal_t::type() const {
    return type_;
}

int64_t literal_t::as_integer() const {
    assert(type_ == literal_type::integer);
    return std::get<int64_t>(value_);
}

bool literal_t::as_boolean() const {
    assert(type_ == literal_type::boolean);
    return std::get<bool>(value_);
}

const std::string& literal_t::as_string() const {
    assert(type_ == literal_type::string);
    return std::get<std::string>(value_);
}

bool literal_t::operator==(const literal_t& rhs) const {
    return type_ == rhs.type_ && value_ == rhs.value_;
}

bool literal_t::operator!=(const literal_t& rhs) const {
    return !(*this == rhs);
}

std::string literal_t::to_string() const {
    switch (type_) {
        case literal_type::integer: return std::to_string(as_integer());
        case literal_type::boolean: return as_boolean() ? "true" : "false";
        case literal_type::string:  return "\"" + as_string() + "\"";
        case literal_type::null:    return "null";
        default:                    return "?";
    }
}

} // namespace components::types
