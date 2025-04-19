#include "table_cost_model.hpp"

namespace components::optimizer::cost {
using namespace components::logical_plan;

// SCAN: пропорционально числу строк
cost_t cost_table_scan(const node_ptr& node) {
    return static_cast<cost_t>(node->estimated_rows());
}

// FILTER: строк меньше, операция дороже
cost_t cost_table_filter(const node_ptr& node) {
    // надо потом учесть селективность
    return static_cast<cost_t>(node->estimated_rows()) * 1.2;
}

} // namespace cost
