#include "predicate_simplification.hpp"
#include <components/expressions/expression_binary.hpp>
#include <components/expressions/expression_constant.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;
using namespace components::expressions;

std::optional<node_ptr> PredicateSimplificationRule::apply(const node_ptr& node) {
    for (auto& child : node->children()) {
        if (auto rewritten = apply(child)) {
            child = *rewritten;
        }
    }

    bool changed = false;
    for (auto& expr : const_cast<std::pmr::vector<expression_ptr>&>(node->expressions())) {
        if (auto bin = std::dynamic_pointer_cast<expression_binary_t>(expr)) {
            if (bin->op() == binary_operator_t::equal && bin->left() == bin->right()) {
                expr = std::make_shared<expression_constant_t>(true);
                changed = true;
            }
        }
    }

    return changed ? std::optional{node} : std::nullopt;
}

}
