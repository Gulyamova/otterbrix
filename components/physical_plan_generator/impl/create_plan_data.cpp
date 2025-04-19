#include "create_plan_data.hpp"
#include <components/logical_plan/node_data.hpp>
#include <components/physical_plan/collection/operators/operator_raw_data.hpp>
#include <components/physical_plan/collection/operators/operator_index_scan.hpp>
#include <components/physical_plan/collection/operators/operator_seq_scan.hpp>

namespace services::collection::planner::impl {

    operators::operator_ptr create_plan_data(const components::logical_plan::node_ptr& node) {
        const auto* data = static_cast<const components::logical_plan::node_data_t*>(node.get());

        size_t rows = node->estimated_rows();
        if (rows < 10'000) {
            return boost::intrusive_ptr(new operators::operator_index_scan_t(data->collection_full_name(), data->expressions()));
        } else {
            return boost::intrusive_ptr(new operators::operator_seq_scan_t(data->collection_full_name(), data->expressions()));
        }
    }

} // namespace services::collection::planner::impl