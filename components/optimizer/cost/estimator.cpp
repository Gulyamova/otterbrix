#include "estimator.hpp"
#include <statistics/statistics.hpp>
#include "utils/memo.hpp"         

namespace components::optimizer::cost {

namespace {

constexpr double kCpuPerRowScan   = 0.5;
constexpr double kRamPerRowScan   = 0.001;
constexpr double kCpuPerRowFilter = 0.2;
constexpr double kRamPerRowFilter = 0.0005;
constexpr double kHashJoinCpuCoef = 0.5;
constexpr double kHashJoinRamCoef = 0.01;

} 

static Memo g_memo;   

cost_t estimate_table_scan(const node_ptr& n) {
    if (g_memo.has(n)) return g_memo.get(n);

    size_t rows = 1000;                                
    if (auto st = n->statistics()) rows = st->row_count;

    cost_t c { rows * kCpuPerRowScan, rows * kRamPerRowScan, static_cast<double>(rows) };
    n->set_estimated_rows(c.rows_out);
    n->set_cpu_cost(c.cpu_cost);
    n->set_ram_cost(c.ram_cost);

    g_memo.put(n, c);
    return c;
}

cost_t estimate_filter(const node_ptr& n) {
    if (g_memo.has(n)) return g_memo.get(n);

    auto input = n->children().front();
    auto inCost = estimate_node_cost(input);           

    double sel  = 0.1;                                  
    if (auto st = n->statistics()) sel = st->selectivity;

    double rowsOut = inCost.rows_out * sel;
    cost_t c { inCost.cpu_cost + inCost.rows_out * kCpuPerRowFilter,
               inCost.ram_cost + inCost.rows_out * kRamPerRowFilter,
               rowsOut };

    n->set_estimated_rows(rowsOut);
    n->set_cpu_cost(c.cpu_cost);
    n->set_ram_cost(c.ram_cost);

    g_memo.put(n, c);
    return c;
}

// ---------- JOIN ----------
cost_t estimate_join(const node_ptr& n) {
    if (g_memo.has(n)) return g_memo.get(n);

    auto l = estimate_node_cost(n->children()[0]);
    auto r = estimate_node_cost(n->children()[1]);

    bool useHash = true;

    double rowsOut = l.rows_out * r.rows_out;         
    double cpu = l.cpu_cost + r.cpu_cost;
    double ram = l.ram_cost + r.ram_cost;

    if (useHash) {
        cpu += rowsOut * kHashJoinCpuCoef;
        ram += rowsOut * kHashJoinRamCoef;
    } else { 
        cpu += rowsOut;               
    }

    cost_t c{cpu, ram, rowsOut};
    n->set_estimated_rows(rowsOut);
    n->set_cpu_cost(cpu);
    n->set_ram_cost(ram);

    g_memo.put(n, c);
    return c;
}

cost_t estimate_node_cost(const node_ptr& n) {
    switch (n->type()) {
        case node_type::data_t:     return estimate_table_scan(n);
        case node_type::function_t: return estimate_filter(n);
        case node_type::join_t:     return estimate_join(n);
        default:                    return {0,0,0};
    }
}

void estimate_node_output_rows(const node_ptr& n) {
    if (auto st = n->statistics())  n->set_estimated_rows(st->row_count);
    else {
        size_t sum = 0;
        for (auto& c : n->children()) sum += c->estimated_rows();
        if (sum == 0) sum = 1000;
        n->set_estimated_rows(sum);
    }
}

} // namespace components::optimizer::cost
