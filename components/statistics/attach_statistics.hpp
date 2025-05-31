#pragma once

#include <components/logical_plan/node.hpp>
#include "collector.hpp"
#include <functional>

namespace statistics {

void attach_statistics_recursively(
        const components::logical_plan::node_ptr& node,
        const std::function<
            const components::table::data_table_t*(const components::collection_full_name_t&)
        >& table_resolver);

} // namespace statistics