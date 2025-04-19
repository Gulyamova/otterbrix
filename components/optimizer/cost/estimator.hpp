#pragma once

#include <logical_plan/node.hpp>

namespace components::optimizer::cost {

void estimate_node_output_rows(const components::logical_plan::node_ptr& node);

} // namespace cost
