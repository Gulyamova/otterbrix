#include "estimator.hpp"
#include <statistics/statistics.hpp>

namespace components::optimizer::cost {
using namespace components::logical_plan;
using namespace statistics;

void estimate_node_output_rows(const node_ptr& node) {
    if (node->statistics()) {
        // статистика есть —> используем row_count
        if (node->statistics()->mode() == data_mode::table) {
            auto stats = std::static_pointer_cast<TableStatistics>(node->statistics());
            node->set_estimated_rows(stats->row_count);
            return;
        }
    }

    // fallback: статистики нет
    size_t estimate = 0;
    for (const auto& child : node->children()) {
        estimate += child->estimated_rows();
    }
    node->set_estimated_rows(estimate > 0 ? estimate : 1000); // дефолт
}

} // namespace cost
