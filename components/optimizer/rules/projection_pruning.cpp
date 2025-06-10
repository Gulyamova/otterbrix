#include "projection_pruning.hpp"
#include "collect_used_expressions.hpp"
#include <components/logical_plan/node.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;

std::optional<node_ptr> ProjectionPruningRule::apply(const node_ptr& node) {
    // собираем все выражения, которые используются выше по дереву
    auto used_exprs = collect_used_expressions(node);

    bool changed = false;

    for (auto& child : node->children()) {
        if (child->type() == node_type::node_data_t && !child->expressions().empty()) {
            std::pmr::vector<expression_ptr> new_exprs(child->resource());

            for (const auto& expr : child->expressions()) {
                if (used_exprs.contains(expr)) {
                    new_exprs.push_back(expr);
                }
            }

            if (new_exprs.size() < child->expressions().size()) {
                child->replace_expressions(std::move(new_exprs));
                changed = true;
            }
        }

        // рекурсивно применяем правило к потомкам
        if (auto rewritten = apply(child)) {
            child = *rewritten;
            changed = true;
        }
    }

    return changed ? node : std::nullopt;
}

} // namespace components::optimizer::rules
