#pragma once

#include <logical_plan/node.hpp>

namespace compnents::optimizer {

components::logical_plan::node_ptr optimize(logical_plan::node_ptr node);

} // namespace optimizer