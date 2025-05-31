#pragma once

namespace components::statistics {

enum class data_mode {
    table,
    document,
    graph
};

struct AbstractStatistics {
    virtual ~AbstractStatistics() = default;
    virtual data_mode mode() const = 0;
    virtual std::size_t row_count()   const = 0;   
    virtual double      selectivity() const = 0;   
};

} // namespace statistics
