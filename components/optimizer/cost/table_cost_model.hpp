#pragma once

#include <components/logical_plan/node.hpp>

namespace components::optimizer::cost {

using cost_t = double;

// temporary: SCAN и FILTER
cost_t cost_table_scan(const components::logical_plan::node_ptr& node);
cost_t cost_table_filter(const components::logical_plan::node_ptr& node);

} // namespace cost