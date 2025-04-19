#pragma once

#include "rule.hpp"
#include <components/vector>

namespace components::optimizer::rules {

components::logical_plan::node_ptr
apply_all(const components::logical_plan::node_ptr& node);

} // namespace optimizer::rules