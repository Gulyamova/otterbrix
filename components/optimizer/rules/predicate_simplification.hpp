#pragma once
#include "rule.hpp"

namespace components::optimizer::rules {

class PredicateSimplificationRule : public Rule {
public:
    std::optional<components::logical_plan::node_ptr> apply(const components::logical_plan::node_ptr& node) override;
};

}
