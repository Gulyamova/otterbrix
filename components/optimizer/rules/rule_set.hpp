#pragma once

#include <components/logical_plan/node.hpp>

namespace components::optimizer::rules {

components::logical_plan::node_ptr apply_all(const components::logical_plan::node_ptr& node);

} // namespace components::optimizer::rules
