#pragma once

#include <string>
#include <variant>
#include <iostream>
#include <cassert>

namespace components::types {

enum class literal_type {
    integer,
    boolean,
    string,
    null
};

class literal_t {
public:
    literal_t();  // null
    literal_t(int64_t v);
    literal_t(bool v);
    literal_t(const std::string& v);
    literal_t(std::string&& v);

    literal_type type() const;

    int64_t as_integer() const;
    bool as_boolean() const;
    const std::string& as_string() const;

    bool operator==(const literal_t& rhs) const;
    bool operator!=(const literal_t& rhs) const;

    std::string to_string() const;

private:
    literal_type type_;
    std::variant<std::monostate, int64_t, bool, std::string> value_;
};

} // namespace components::types
