#include "join_strategy.hpp"

namespace components::optimizer::cost {

using namespace components::logical_plan;

join_strategy_t choose_join_strategy(const node_ptr& left, const node_ptr& right) {
    // обе таблицы маленькие — nested_loop
    size_t left_rows = left->estimated_rows();
    size_t right_rows = right->estimated_rows();

    if (left_rows < 1000 && right_rows < 1000) {
        return join_strategy_t::nested_loop;
    }

    return join_strategy_t::hash;
}

std::string to_string(join_strategy_t strategy) {
    switch (strategy) {
        case join_strategy_t::hash: return "Hash Join";
        case join_strategy_t::nested_loop: return "Nested Loop";
        default: return "Unknown";
    }
}

} // namespace cost
