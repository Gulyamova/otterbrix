#include <catch2/catch.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_aggregate.hpp>
#include <components/logical_plan/node_sort.hpp>
#include <optimizer/optimizer.hpp>
#include <statistics/attach_statistics.hpp>

using namespace components::logical_plan;
using namespace optimizer;
using namespace statistics;

TEST_CASE("Join + Aggregate + Sort get optimized correctly") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});

    A->set_statistics(std::make_shared<TableStatistics>(2000));
    B->set_statistics(std::make_shared<TableStatistics>(500));

    auto join = make_node_join(nullptr, {"db", "X"});
    join->append_child(A);
    join->append_child(B);

    auto agg = make_node_aggregate(nullptr, {"db", "X"});
    agg->append_child(join);

    auto sort = make_node_sort(nullptr, {"db", "X"});
    sort->append_child(agg);

    auto optimized = optimize(sort);

    REQUIRE(optimized->type() == node_type::sort_t);
    REQUIRE(optimized->children()[0]->type() == node_type::aggregate_t);
    REQUIRE(optimized->children()[0]->children()[0]->type() == node_type::join_t);
}
