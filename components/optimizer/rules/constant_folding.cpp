#include "constant_folding.hpp"
#include <components/expressions/expression_constant.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;
using namespace components::expressions;

std::optional<node_ptr> ConstantFoldingRule::apply(const node_ptr& node) {
    for (auto& child : node->children()) {
        if (auto rewritten = apply(child)) {
            child = *rewritten;
        }
    }

    bool changed = false;
    for (auto& expr : const_cast<std::pmr::vector<expression_ptr>&>(node->expressions())) {
        if (expr->is_constant()) continue;
        if (auto folded = expr->try_fold_constant()) {
            expr = std::make_shared<expression_constant_t>(folded.value());
            changed = true;
        }
    }

    return changed ? std::optional{node} : std::nullopt;
}

}
