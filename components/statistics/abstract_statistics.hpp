#pragma once

namespace components::statistics {

enum class data_mode {
    table,
    document,
    graph
};

struct AbstractStatistics {
    virtual data_mode mode() const = 0;
    virtual ~AbstractStatistics() = default;
};

} // namespace statistics
