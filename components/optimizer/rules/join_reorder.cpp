#include "join_reorder.hpp"
#include <optimizer/cost/estimator.hpp>
#include <components/logical_plan/node_join.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;
using cost::estimate_node_cost;

std::optional<node_ptr> JoinReorderRule::apply(const node_ptr& node) {
    bool changed = false;

    // Сначала рекурсивно применяем к потомкам
    for (auto& child : node->children()) {
        if (auto rewritten = apply(child)) {
            child = *rewritten;
            changed = true;
        }
    }

    if (node->type() != node_type::join_t || node->children().size() < 2) {
        return changed ? std::optional{node} : std::nullopt;
    }

    auto& inputs = node->children();
    const size_t n = inputs.size();

    std::vector<bool> used(n, false);

    // Выбираем начальный узел с минимальной стоимостью
    size_t best_idx = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (size_t i = 0; i < n; ++i) {
        double cost = estimate_node_cost(inputs[i]).total();
        if (cost < best_cost) {
            best_cost = cost;
            best_idx = i;
        }
    }

    node_ptr current = inputs[best_idx];
    used[best_idx] = true;

    for (size_t step = 1; step < n; ++step) {
        double min_cost = std::numeric_limits<double>::max();
        size_t next_idx = -1;

        for (size_t i = 0; i < n; ++i) {
            if (used[i]) continue;

            auto candidate = make_node_join(node->resource(), current->collection_full_name());
            candidate->append_child(current);
            candidate->append_child(inputs[i]);

            double cost = estimate_node_cost(candidate).total();
            if (cost < min_cost) {
                min_cost = cost;
                next_idx = i;
            }
        }

        auto new_join = make_node_join(node->resource(), current->collection_full_name());
        new_join->append_child(current);
        new_join->append_child(inputs[next_idx]);

        current = new_join;
        used[next_idx] = true;
        changed = true;
    }

    return changed ? std::optional{current} : std::nullopt;
}

} // namespace components::optimizer::rules