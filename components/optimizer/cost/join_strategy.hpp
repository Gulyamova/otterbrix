#pragma once

#include <components/logical_plan/node.hpp>
#include <string>

namespace components::optimizer::cost {

enum class join_strategy_t {
    hash,
    nested_loop
};

join_strategy_t choose_join_strategy(const components::logical_plan::node_ptr& left,
                                     const components::logical_plan::node_ptr& right);

std::string to_string(join_strategy_t strategy);

} // namespace cost
