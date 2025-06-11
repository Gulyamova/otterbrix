#include "statistics.hpp"
#include <algorithm>

namespace components::statistics {

ColumnStatistics::ColumnStatistics(std::pmr::memory_resource* res, size_t bins)
    : histogram(res), num_bins(bins), cardinality(0) {
    histogram.resize(num_bins, 0);
}

void ColumnStatistics::update(int64_t value) {
    if (value < min_value) min_value = value;
    if (value > max_value) max_value = value;
}

void ColumnStatistics::update_with_histogram(int64_t value) {
    update(value);
    if (max_value > min_value) {
        size_t bin = static_cast<size_t>(
            double(value - min_value) / (max_value - min_value + 1) * num_bins
        );
        bin = std::min(bin, num_bins - 1);
        ++histogram[bin];
    }
}

TableStatistics::TableStatistics(std::pmr::memory_resource* res)
    : AbstractStatistics()
    , row_count(0)
    , columns(std::pmr::polymorphic_allocator<std::pair<const std::string, ColumnStatistics>>(res)) {}

} // namespace components::statistics
