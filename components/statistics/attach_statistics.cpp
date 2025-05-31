#include "attach_statistics.hpp"
#include "statistics.hpp"                               
#include <components/table/data_table.hpp>

using namespace components;
using namespace components::logical_plan;

namespace components::statistics {

void attach_statistics_recursively(
        const node_ptr& node,
        const std::function<const table::data_table_t*(const collection_full_name_t&)>& table_resolver)
{
    for (const auto& child : node->children()) {
        attach_statistics_recursively(child, table_resolver);
    }

    if (node->type() != node_type::data_t) {
        return;
    }

    const auto* tbl = table_resolver(node->collection_full_name());
    std::shared_ptr<AbstractStatistics> stats;

    if (tbl) {
        auto ts = std::make_shared<TableStatistics>(node->resource());
        ts->set_row_count(tbl->total_rows());
        ts->set_selectivity(1.0);        
        stats = ts;
    } else {
        auto ts = std::make_shared<TableStatistics>(node->resource());
        ts->set_row_count(1000);
        ts->set_selectivity(1.0);
        stats = ts;
    }

    node->set_statistics(stats);
    node->set_estimated_rows(stats->row_count());
}

} // namespace components::statistics
