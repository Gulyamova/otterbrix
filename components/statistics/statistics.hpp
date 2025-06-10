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
    explicit TableStatistics(std::pmr::memory_resource* res);
    data_mode  mode() const override       { return data_mode::table; }
    std::size_t row_count()   const override { return row_count_; }
    double      selectivity() const override { return selectivity_; }
    void set_row_count(std::size_t v) { row_count_  = v; }
    void set_selectivity(double  v)   { selectivity_ = v; }

    std::pmr::unordered_map<std::string, ColumnStatistics> columns;

    private:
    std::size_t row_count_  = 1'000;   //  дефолт
    double      selectivity_ = 1.0;    
};

} // namespace statistics