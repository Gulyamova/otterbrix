#include "projection_pruning.hpp"
#include "collect_used_expressions.hpp"
#include <components/logical_plan/node.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;

std::optional<node_ptr> ProjectionPruningRule::apply(const node_ptr& node) {
    for (auto& child : node->children()) {
        if (auto rewritten = apply(child)) {
            child = *rewritten;
        }
    }

    if (node->type() == node_type::data_t && !node->expressions().empty()) {
        auto used = collect_used_expressions(node);
        std::pmr::vector<expression_ptr> new_exprs(node->resource());

        for (const auto& expr : node->expressions()) {
            if (used.contains(expr)) {
                new_exprs.push_back(expr);
            }
        }

        if (new_exprs.size() < node->expressions().size()) {
            auto& exprs = const_cast<std::pmr::vector<expression_ptr>&>(node->expressions());
            exprs = std::move(new_exprs);
            return node;
        }
    }

    return std::nullopt;
}

} // namespace components::optimizer::rules
