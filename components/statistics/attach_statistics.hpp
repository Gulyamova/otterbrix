#pragma once

#include <logical_plan/node.hpp>
#include "collector.hpp"

namespace statistics {

void attach_statistics_recursively(const components::logical_plan::node_ptr& node);

} // namespace statistics