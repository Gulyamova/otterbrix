#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <limits>
#include <cstddef>
#include <vector>
#include <string_view>
#include <memory_resource>

namespace components::statistics {

enum class data_mode {
    table,
    document,
    graph
};

// Абстрактный интерфейс статистики
struct AbstractStatistics {
    virtual ~AbstractStatistics() = default;
    virtual data_mode mode() const = 0;
    virtual std::size_t row_count() const = 0;
    virtual double selectivity() const = 0;
};

// Статистика по колонке (гистограмма и min/max)
struct ColumnStatistics {
    int64_t min_value = std::numeric_limits<int64_t>::max();
    int64_t max_value = std::numeric_limits<int64_t>::min();
    std::pmr::vector<size_t> histogram;
    size_t num_bins = 10;

    ColumnStatistics(std::pmr::memory_resource* res, size_t bins = 10)
        : histogram(res), num_bins(bins) {
        histogram.resize(num_bins, 0);
    }

    void update(int64_t value) {
        if (value < min_value) min_value = value;
        if (value > max_value) max_value = value;
    }

    void update_with_histogram(int64_t value) {
        update(value);
        if (max_value > min_value) {
            size_t bin = static_cast<size_t>(
                double(value - min_value) / (max_value - min_value + 1) * num_bins
            );
            bin = std::min(bin, num_bins - 1);
            ++histogram[bin];
        }
    }
};

// Статистика по всей таблице
struct TableStatistics : public AbstractStatistics {
    std::size_t rows = 0;
    std::pmr::unordered_map<std::string, ColumnStatistics> columns;

    explicit TableStatistics(std::pmr::memory_resource* res)
        : columns(std::pmr::polymorphic_allocator<std::pair<const std::string, ColumnStatistics>>(res)) {}

    data_mode mode() const override {
        return data_mode::table;
    }

    std::size_t row_count() const override {
        return rows;
    }

    double selectivity() const override {
        return rows > 0 ? 0.1 : 1.0;
    }

    ColumnStatistics& column_stats(const std::string& name) {
        return columns.try_emplace(name, columns.get_allocator().resource()).first->second;
    }
};

} // namespace components::statistics
