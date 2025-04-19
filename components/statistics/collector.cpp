#include "collector.hpp"
#include <components/vector/vector_operations.hpp>
#include <unordered_set>

namespace components::statistics {

StatisticsCollector::StatisticsCollector(std::pmr::memory_resource* resource)
    : resource_(resource) {}

ColumnStatistics StatisticsCollector::collect_for_column(const vector::vector_t& column) {
    ColumnStatistics stats(resource_);
    std::unordered_set<int64_t> unique;

    auto* ptr = column.data<int64_t>();
    for (size_t i = 0; i < column.size(); ++i) {
        int64_t value = ptr[i];
        stats.update_with_histogram(value);
        unique.insert(value);
    }

    stats.cardinality = unique.size();
    return stats;
}

std::shared_ptr<TableStatistics> StatisticsCollector::collect_for_table(const table::data_table_t& table) {
    auto stats = std::make_shared<TableStatistics>(resource_);
    stats->row_count = table.total_rows();

    table.scan_table_segment(0, stats->row_count, [&](vector::data_chunk_t& chunk) {
        for (size_t i = 0; i < chunk.column_count(); ++i) {
            auto& column = chunk.get_column(i);
            column.flatten();

            std::string col_name = std::to_string(i);
            auto col_stats = collect_for_column(column);
            stats->columns[col_name] = col_stats;
        }
    });

    return stats;
}

} // namespace statistics