#pragma once

#include <components/logical_plan/node.hpp>
#include <optional>

namespace components::optimizer::rules {

struct Rule {
    virtual ~Rule() = default;

    virtual std::optional<logical_plan::node_ptr>
    apply(logical_plan::node_ptr node) = 0;
};

} // namespace components::optimizer::rules
