#pragma once

#include "statistics.hpp"
#include <table/data_table.hpp>

namespace components::statistics {

class StatisticsCollector {
public:
    explicit StatisticsCollector(std::pmr::memory_resource* resource);

    std::shared_ptr<TableStatistics> collect_for_table(const components::table::data_table_t& table);

private:
    std::pmr::memory_resource* resource_;
    ColumnStatistics collect_for_column(const components::vector::vector_t& column);
};

} // namespace statistics
