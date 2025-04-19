#pragma once

#include "rule.hpp"

namespace optimizer::rules {

// LIMIT ближе к источнику
class LimitPushdownRule : public Rule {
public:
    std::optional<components::logical_plan::node_ptr>
    apply(const components::logical_plan::node_ptr& node) override;
};

} // namespace optimizer::rules
