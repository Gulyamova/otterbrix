#include "estimator.hpp"
#include <components/logical_plan/node_join.hpp>

namespace components::optimizer::cost {
using namespace components::logical_plan;

join_strategy_t choose_join_strategy(const node_ptr& left, const node_ptr& right) {
    struct cand { join_strategy_t type; cost_t c; };

    std::array<cand, 5> v;

    auto try_strategy = [&](join_strategy_t strategy, auto estimator_func) {
        auto join_node = make_node_join(left->resource(), {database_name_t(), collection_name_t()}, join_type::inner);
        join_node->append_child(left);
        join_node->append_child(right);
        join_node->set_strategy(strategy);
        return cand{strategy, estimator_func(join_node)};
    };

    v[0] = try_strategy(join_strategy_t::hash,               estimate_join_hash);
    v[1] = try_strategy(join_strategy_t::nested_loop,        estimate_join_nested_loop);
    v[2] = try_strategy(join_strategy_t::merge,              estimate_join_merge);
    v[3] = try_strategy(join_strategy_t::index_nested_loop,  estimate_join_index_nl);
    v[4] = try_strategy(join_strategy_t::grace_hash,         estimate_join_grace_hash);

    return std::min_element(v.begin(), v.end(),
            [](const cand& a, const cand& b){ return a.c.total() < b.c.total(); })
           ->type;
}

std::string to_string(join_strategy_t strategy) {
    switch (strategy) {
        case join_strategy_t::hash:               return "Hash Join";
        case join_strategy_t::nested_loop:        return "Nested Loop";
        case join_strategy_t::merge:              return "Merge Join";
        case join_strategy_t::index_nested_loop:  return "Index Nested Loop";
        case join_strategy_t::grace_hash:         return "Grace Hash Join";
        default:                                  return "Unknown";
    }
}

} // namespace components::optimizer::cost
