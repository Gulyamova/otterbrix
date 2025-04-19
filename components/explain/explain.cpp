#include "explain.hpp"
#include <sstream>

namespace explain {
using namespace components::logical_plan;

std::string to_string(const node_ptr& node, int indent) {
    std::stringstream ss;
    std::string pad(indent, ' ');

    ss << pad << node->to_string();
    ss << " [rows=" << node->estimated_rows() << "]\n";

    for (const auto& child : node->children()) {
        ss << to_string(child, indent + 2);
    }

    return ss.str();
}

} // namespace explain
