#pragma once

#include "statistics.hpp"

namespace components::statistics {

double estimate_selectivity_gt(const ColumnStatistics& stats, int64_t threshold);
double estimate_selectivity_eq(const ColumnStatistics& stats, int64_t value);

} // namespace statistics

