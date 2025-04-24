#include "estimator.hpp"

namespace components::optimizer::cost {

using namespace components::logical_plan;

join_strategy_t choose_join_strategy(const node_ptr& left, const node_ptr& right) {
    // Оценка левого и правого узлов
    cost_t left_cost = estimate_node_cost(left);
    cost_t right_cost = estimate_node_cost(right);

    // Оценка разных стратегий
    double hash_cost = left_cost.cpu_cost + right_cost.cpu_cost + left_cost.ram_cost + right_cost.ram_cost;
    double nested_loop_cost = left_cost.rows_out * right_cost.rows_out * 0.5 + left_cost.cpu_cost + right_cost.cpu_cost;

    if (hash_cost < nested_loop_cost) {
        return join_strategy_t::hash; 
    } else {
        return join_strategy_t::nested_loop;
    }
}

std::string to_string(join_strategy_t strategy) {
    switch (strategy) {
        case join_strategy_t::hash: return "Hash Join";
        case join_strategy_t::nested_loop: return "Nested Loop";
        default: return "Unknown";
    }
}

} // namespace cost
