#pragma once

#include "abstract_statistics.hpp"
#include <cstdint>
#include <string>
#include <limits>
#include <vector>
#include <unordered_map>

namespace components::statistics {

struct ColumnStatistics {
    size_t cardinality = 0;
    int64_t min_value = std::numeric_limits<int64_t>::max();
    int64_t max_value = std::numeric_limits<int64_t>::min();
    std::pmr::vector<size_t> histogram;
    size_t num_bins = 10;
    double selectivity = 1.0;

    ColumnStatistics(std::pmr::memory_resource* res, size_t bins = 10);

    void update(int64_t value);
    void update_with_histogram(int64_t value);
};

struct TableStatistics : public AbstractStatistics {
    std::pmr::unordered_map<std::string, ColumnStatistics> columns;
    size_t row_count = 0;

    TableStatistics(std::pmr::memory_resource* res);

    data_mode mode() const override {
        return data_mode::table;
    }
};

} // namespace statistics