#include "rule_set.hpp"
#include "rule.hpp"

#include "filter_pushdown.hpp"
#include "projection_pruning.hpp"
#include "constant_folding.hpp"
#include "predicate_simplification.hpp"
#include "join_reorder.hpp"

#include <vector>
#include <memory>
#include <iostream>

namespace components::optimizer::rules {
using namespace components::logical_plan;

static std::vector<std::unique_ptr<Rule>> all_rules = {
    std::make_unique<FilterPushdownRule>(),
    std::make_unique<ProjectionPruningRule>(),
    std::make_unique<ConstantFoldingRule>(),
    std::make_unique<PredicateSimplificationRule>(),
    std::make_unique<JoinReorderRule>()
};

node_ptr apply_all(const node_ptr& node) {
    for (auto& rule : all_rules) {
        if (auto rewritten = rule->apply(node)) {
            std::cout << "[RULE] Applied: " << typeid(*rule).name() << std::endl;
            return apply_all(*rewritten); 
        }
    }
    return node;  
}

} // namespace components::optimizer::rules
