#pragma once

#include <memory>
#include <components/logical_plan/node.hpp>

namespace components::optimizer::cost {

struct cost_t {
    double cpu_cost;
    double ram_cost;
    double rows_out;

    double total() const {
        return cpu_cost + ram_cost * 0.01; 
    }
};

void estimate_node_output_rows(const node_ptr& node);  
cost_t estimate_node_cost(const node_ptr& node); 

}  // namespace components::optimizer::cost
