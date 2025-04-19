#include "limit_pushdown.hpp"
#include <components/logical_plan/node_limit.hpp>
#include <components/logical_plan/node_data.hpp>

namespace components::optimizer::rules {

using namespace components::logical_plan;

std::optional<node_ptr> LimitPushdownRule::apply(const node_ptr& node) {
    // Условие: LIMIT над SCAN (data_t)
    if (node->type() != node_type::limit_t || node->children().size() != 1) {
        return std::nullopt;
    }

    auto& child = node->children().front();
    if (child->type() != node_type::data_t) {
        return std::nullopt;
    }

    // новый SCAN с лимитом в expressions
    auto resource = node->resource();
    auto new_scan = make_node_data(resource, child->collection_full_name());

    // Прокидываем выражения
    new_scan->append_expressions(node->expressions());

    return new_scan;
}

} // namespace optimizer::rules
