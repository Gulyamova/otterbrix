#pragma once

#include <components/logical_plan/node.hpp>
#include <string>

namespace components::explain {

std::string to_string(const components::logical_plan::node_ptr& node, int indent = 0);

} // namespace explain