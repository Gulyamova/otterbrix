#include "filter_pushdown.hpp"
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_function.hpp>

namespace optimizer::rules {
using namespace components::logical_plan;

std::optional<node_ptr> FilterPushdownRule::apply(const node_ptr& node) {
    // Условие: фильтр над источником данных
    if (node->type() != node_type::function_t) {
        return std::nullopt;
    }

    if (node->children().size() != 1) {
        return std::nullopt;
    }

    auto& child = node->children().front();
    if (child->type() != node_type::data_t) {
        return std::nullopt;
    }

    // Создаём новый узел-фильтр над источником
    auto resource = node->resource();
    auto new_scan = make_node_data(resource, child->collection_full_name());

    // Прокидываем выражение фильтра
    new_scan->append_expressions(node->expressions());

    // Возвращаем новый узел
    return new_scan;
}

} // namespace optimizer::rules