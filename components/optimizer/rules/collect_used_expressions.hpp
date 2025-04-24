#pragma once

#include <unordered_set>
#include <components/logical_plan/node.hpp>

namespace components::optimizer::rules {

using components::logical_plan::node_ptr;
using components::expressions::expression_ptr;

std::unordered_set<expression_ptr> collect_used_expressions(const node_ptr& node);

} // namespace components::optimizer::rules
