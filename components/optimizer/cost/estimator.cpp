#include "estimator.hpp"
#include <statistics/statistics.hpp>
#include "utils/memo.hpp"
#include <cmath>
#include <limits>         

namespace components::optimizer::cost {

namespace {

constexpr double kCpuPerRowScan   = 0.5;
constexpr double kRamPerRowScan   = 0.001;
constexpr double kCpuPerRowFilter = 0.2;
constexpr double kRamPerRowFilter = 0.0005;
constexpr double kHashJoinCpuCoef = 0.5;
constexpr double kHashJoinRamCoef = 0.01;
constexpr double kNestedLoopCmp   = 0.5;   
constexpr double kSpillFactor     = 4.0; 

} 

static Memo g_memo;   

cost_t estimate_table_scan(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

    size_t rows = 1000;                                
    if (auto st = n->statistics()) rows = st->row_count();

    cost_t c { rows * kCpuPerRowScan, rows * kRamPerRowScan, static_cast<double>(rows) };
    n->set_estimated_rows(c.rows_out);
    n->set_cpu_cost(c.cpu_cost);
    n->set_ram_cost(c.ram_cost);

    g_memo.put_cost(n, c);
    return c;
}

cost_t estimate_filter(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

    auto input = n->children().front();
    auto inCost = estimate_node_cost(input);           

    double sel  = 0.1;                                  
    if (auto st = n->statistics()) sel = st->selectivity();

    double rowsOut = inCost.rows_out * sel;
    cost_t c { inCost.cpu_cost + inCost.rows_out * kCpuPerRowFilter,
               inCost.ram_cost + inCost.rows_out * kRamPerRowFilter,
               rowsOut };

    n->set_estimated_rows(c.rowsOut);
    n->set_cpu_cost(c.cpu_cost);
    n->set_ram_cost(c.ram_cost);

    g_memo.put_cost(n, c);
    return c;
}

// ---------- JOIN ----------
cost_t estimate_join_hash(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

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
    n->set_estimated_rows(c.rowsOut);
    n->set_cpu_cost(c.cpu_cost);
    n->set_ram_cost(c.ram_cost);

    g_memo.put_cost(n, c);
    return c;
}

// ---------- JOIN - Nested-Loop ----------
cost_t estimate_join_nested_loop(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

    auto L = estimate_node_cost(n->children()[0]);
    auto R = estimate_node_cost(n->children()[1]);

    double rows = L.rows_out * R.rows_out;
    cost_t c { L.cpu_cost + R.cpu_cost + rows * kNestedLoopCmp,
               L.ram_cost + R.ram_cost,
               rows };
    n->set_cost(c); g_memo.put_cost(n,c); return c;
}

// ---------- JOIN - Merge-Join ----------
cost_t estimate_join_merge(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

    auto L = estimate_node_cost(n->children()[0]);
    auto R = estimate_node_cost(n->children()[1]);

    bool sorted = false;                       // TODO: анализ Sort-узлов
    double sortPenalty = sorted ? 0
                                : L.rows_out*std::log2(L.rows_out)
                                + R.rows_out*std::log2(R.rows_out);

    cost_t c { L.cpu_cost + R.cpu_cost + sortPenalty,
               L.ram_cost + R.ram_cost,
               L.rows_out * R.rows_out };
    n->set_cost(c); g_memo.put_cost(n,c); return c;
}

// ---------- JOIN - Index-Nested-Loop ----------
cost_t estimate_join_index_nl(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

    bool hasIndex = false;                     // TODO: спросить каталог
    if (!hasIndex)
        return { std::numeric_limits<double>::infinity(),
                 std::numeric_limits<double>::infinity(), 0 };

    auto L = estimate_node_cost(n->children()[0]);
    auto R = estimate_node_cost(n->children()[1]);

    cost_t c { L.cpu_cost + L.rows_out * std::log2(R.rows_out),
               L.ram_cost,
               L.rows_out * R.rows_out };
    n->set_cost(c); g_memo.put_cost(n,c); return c;
}
// ---------- JOIN - Grace-Hash ----------
cost_t estimate_join_grace_hash(const node_ptr& n) {
    if (g_memo.has_cost(n)) return g_memo.get_cost(n);

    auto L = estimate_node_cost(n->children()[0]);
    auto R = estimate_node_cost(n->children()[1]);

    double rows = L.rows_out * R.rows_out;
    cost_t c { L.cpu_cost + R.cpu_cost,
               (L.ram_cost + R.ram_cost) * kSpillFactor,
               rows };
    n->set_cost(c); g_memo.put_cost(n,c); return c;
}


cost_t estimate_node_cost(const node_ptr& n) {
    if (auto st = n->statistics()) {
        n->set_estimated_rows(st->row_count);
        return;
    }

    if (n->type() == node_type::join_t && n->children().size() >= 2) {      // ADD
        auto left  = n->children()[0]->estimated_rows();
        auto right = n->children()[1]->estimated_rows();
        n->set_estimated_rows(static_cast<size_t>(left * right * 0.1));     // 10 % эвристика
        return;
    }

    size_t sum = 0;
    for (auto& c : n->children()) sum += c->estimated_rows();
    if (sum == 0) sum = 1000;
    n->set_estimated_rows(sum);
}

} // namespace components::optimizer::cost
