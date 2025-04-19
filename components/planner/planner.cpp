#include "planner.hpp"
#include <components/statistics/attach_statistics.hpp> 
#include <components/optimizer/optimizer.hpp>

namespace components::planner {

    auto planner_t::create_plan(std::pmr::memory_resource* resource, logical_plan::node_ptr node)
        -> logical_plan::node_ptr {
        assert(resource && node);

        statistics::attach_statistics_recursively(node);

        return optimizer::optimize(node);;
    }

} // namespace components::planner
