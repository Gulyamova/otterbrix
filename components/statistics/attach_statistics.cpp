#include "attach_statistics.hpp"

using namespace components::logical_plan;

namespace components::statistics {

// TODO: реализовать функцию получения таблицы по имени
const table::data_table_t* resolve_table_from_catalog(const collection_full_name_t& name) {
    // Заглушка: в реальной системе нужно искать в session/catalog
    return nullptr;
}

void attach_statistics_recursively(const node_ptr& node) {
    for (const auto& child : node->children()) {
        attach_statistics_recursively(child);
    }

    if (node->type() == node_type::data_t) {
        auto resource = node->resource();

        const auto& collection_name = node->collection_full_name();
        const table::data_table_t* table = resolve_table_from_catalog(collection_name);
        if (!table) {
            return;
        }

        StatisticsCollector collector(resource);
        auto stats = collector.collect_for_table(*table);
        node->set_statistics(stats);
        node->set_estimated_rows(stats->row_count);
    }
}
}