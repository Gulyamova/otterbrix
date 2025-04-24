#include "collect_used_expressions.hpp"

namespace components::optimizer::rules {

std::unordered_set<expression_ptr> collect_used_expressions(const node_ptr& node) {
    std::unordered_set<expression_ptr> result;

    for (const auto& expr : node->expressions()) {
        result.insert(expr);
    }

    for (const auto& child : node->children()) {
        auto child_used = collect_used_expressions(child);
        result.insert(child_used.begin(), child_used.end());
    }

    return result;
}

} // namespace components::optimizer::rules
