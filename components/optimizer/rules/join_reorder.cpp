#include "join_reorder.hpp"
#include <optimizer/cost/estimator.hpp>
#include <components/logical_plan/node_join.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;
using cost::estimate_node_cost;

std::optional<node_ptr> JoinReorderRule::apply(const node_ptr& node) {
    for (auto& child : node->children()) {
        if (auto rewritten = apply(child)) {
            child = *rewritten;
        }
    }

    if (node->type() != node_type::join_t || node->children().size() < 2) {
        return std::nullopt;
    }

    std::vector<node_ptr> inputs = node->children();
    std::vector<bool> used(inputs.size(), false);

    int start_idx = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (int i = 0; i < inputs.size(); ++i) {
        double cost = estimate_node_cost(inputs[i]).total();
        if (cost < best_cost) {
            best_cost = cost;
            start_idx = i;
        }
    }

    node_ptr current = inputs[start_idx];
    used[start_idx] = true;

    for (int step = 1; step < inputs.size(); ++step) {
        double min_cost = std::numeric_limits<double>::max();
        int next_idx = -1;
        for (int i = 0; i < inputs.size(); ++i) {
            if (used[i]) continue;

            auto temp_join = make_node_join(node->resource(), current->collection_full_name());
            temp_join->append_child(current);
            temp_join->append_child(inputs[i]);

            double cost = estimate_node_cost(temp_join).total();
            if (cost < min_cost) {
                min_cost = cost;
                next_idx = i;
            }
        }

        auto next_join = make_node_join(node->resource(), current->collection_full_name());
        next_join->append_child(current);
        next_join->append_child(inputs[next_idx]);
        current = next_join;
        used[next_idx] = true;
    }

    return std::optional{current};
}

}
