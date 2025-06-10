#include "limit_pushdown.hpp"
#include <components/logical_plan/node_limit.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_sort.hpp>
#include <components/logical_plan/node_aggregate.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;

std::optional<node_ptr> LimitPushdownRule::apply(const node_ptr& node) {
    if (node->type() != node_type::limit_t || node->children().size() != 1) {
        return std::nullopt;
    }

    const auto& child = node->children().front();
    if (child->type() == node_type::node_data_t ||
        child->type() == node_type::node_sort_t ||
        child->type() == node_type::node_aggregate_t) {
        
        auto new_limit = std::make_shared<node_limit>(*std::static_pointer_cast<node_limit>(node));
        new_limit->clear_children();
        new_limit->append_child(child);

        return new_limit;
    }

    return std::nullopt;
}

} // namespace components::optimizer::rules

