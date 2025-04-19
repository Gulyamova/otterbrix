#include "estimator.hpp"
#include <algorithm>

namespace components::statistics {

double estimate_selectivity_gt(const ColumnStatistics& stats, int64_t threshold) {
    if (stats.histogram.empty() || stats.max_value <= stats.min_value) {
        if (threshold >= stats.max_value) return 0.0;
        if (threshold <= stats.min_value) return 1.0;
        return double(stats.max_value - threshold) / (stats.max_value - stats.min_value + 1);
    }

    size_t total = 0, matching = 0;
    size_t num_bins = stats.histogram.size();
    for (size_t i = 0; i < num_bins; ++i) {
        total += stats.histogram[i];
        int64_t bin_start = stats.min_value + (stats.max_value - stats.min_value) * i / num_bins;
        if (bin_start > threshold) {
            matching += stats.histogram[i];
        }
    }
    return total > 0 ? double(matching) / total : 1.0;
}

double estimate_selectivity_eq(const ColumnStatistics& stats, int64_t value) {
    if (value < stats.min_value || value > stats.max_value) return 0.0;
    return 1.0 / std::max<size_t>(stats.cardinality, 1);
}

} // namespace statistics
