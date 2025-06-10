#include "filter_pushdown.hpp"
#include <components/logical_plan/node_match.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_sort.hpp>
#include <components/logical_plan/node_aggregate.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;

// Вспомогательная функция для обработки JOIN
node_ptr push_filter_into_join(node_ptr join_node, const expression_ptr& expr, memory::resource* resource) {
    auto join = std::static_pointer_cast<node_join>(join_node);
    auto new_join = std::make_shared<node_join>(*join);
    new_join->clear_children();
    for (const auto& child : join->children()) {
        if (child->type() == node_type::node_data_t) {
            auto match = make_node_match(resource, child->collection_full_name(), expr);
            match->append_child(child);
            new_join->append_child(match);
        } else {
            new_join->append_child(child);
        }
    }
    return new_join;
}

std::optional<node_ptr> FilterPushdownRule::apply(const node_ptr& node) {
    if (node->type() != node_type::node_match_t || node->children().size() != 1) {
        return std::nullopt;
    }

    const auto& child = node->children().front();
    auto exprs = node->expressions();
    if (exprs.empty()) {
        return std::nullopt;
    }

    auto resource = node->resource();

    switch (child->type()) {
        case node_type::node_data_t:
        case node_type::node_sort_t:
        case node_type::node_aggregate_t: {
            auto match = make_node_match(resource, child->collection_full_name(), exprs.front());
            match->append_child(child);
            return match;
        }
        case node_type::node_join_t:
            return push_filter_into_join(child, exprs.front(), resource);
        default:
            return std::nullopt;
    }
}

} // namespace optimizer::rules
