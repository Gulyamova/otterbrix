#include "rule_set.hpp"
#include "filter_pushdown.hpp"

namespace optimizer::rules {
using namespace components::logical_plan;

static std::vector<std::unique_ptr<Rule>> all_rules = {
    std::make_unique<FilterPushdownRule>()
};

node_ptr apply_all(const node_ptr& node) {
    for (auto& rule : all_rules) {
        if (auto rewritten = rule->apply(node)) {
            return apply_all(*rewritten);
        }
    }
    return node; 
}

} // namespace optimizer::rules
