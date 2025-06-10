#include "collect_used_expressions.hpp"

namespace components::optimizer::rules {

using namespace components::logical_plan;
using namespace components::expressions;

std::unordered_set<expression_ptr> collect_used_expressions(const node_ptr& node) {
    std::unordered_set<expression_ptr> used;

    for (const auto& expr : node->expressions()) {
        used.insert(expr);
    }

    for (const auto& child : node->children()) {
        const auto child_exprs = collect_used_expressions(child);
        used.insert(child_exprs.begin(), child_exprs.end());
    }

    return used;
}

} // namespace components::optimizer::rules
