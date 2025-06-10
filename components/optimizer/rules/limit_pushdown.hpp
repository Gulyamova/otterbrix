#pragma once

#include "rule.hpp"

namespace components::optimizer::rules {

class LimitPushdownRule : public Rule {
public:
    std::optional<logical_plan::node_ptr> apply(const logical_plan::node_ptr& node) override;
};

} // namespace optimizer::rules
