#include "constant_folding.hpp"
#include <components/expressions/expression_constant.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;
using namespace components::expressions;

std::optional<node_ptr> ConstantFoldingRule::apply(const node_ptr& node) {
    bool changed = false;

    for (auto& child : node->children()) {
        if (auto rewritten = apply(child)) {
            child = *rewritten;
            changed = true;
        }
    }

    std::pmr::vector<expression_ptr> new_exprs(node->resource());
    for (const auto& expr : node->expressions()) {
        if (expr->is_constant()) {
            new_exprs.push_back(expr);
        } else if (auto folded = expr->try_fold_constant()) {
            new_exprs.push_back(std::make_shared<expression_constant_t>(folded.value()));
            changed = true;
        } else {
            new_exprs.push_back(expr);
        }
    }

    if (changed) {
        node->replace_expressions(std::move(new_exprs));
        return node;
    }

    return std::nullopt;
}

} // namespace components::optimizer::rules
