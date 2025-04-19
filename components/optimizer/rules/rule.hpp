#pragma once

#include <components/logical_plan/node.hpp"
#include <optional>

namespace components::optimizer::rules {

class Rule {
public:
    virtual ~Rule() = default;

    virtual std::optional<components::logical_plan::node_ptr>
    apply(const components::logical_plan::node_ptr& node) = 0;
};

} // namespace optimizer::rules