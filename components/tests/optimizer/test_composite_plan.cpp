#include <catch2/catch.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_aggregate.hpp>
#include <components/logical_plan/node_data.hpp>
#include <statistics/attach_statistics.hpp>
#include <optimizer/optimizer.hpp>

using namespace components::logical_plan;
using namespace optimizer;

TEST_CASE("Optimizer rewrites composite tree: join -> agg") {
    auto left = make_node_data(nullptr, {"db", "left"});
    auto right = make_node_data(nullptr, {"db", "right"});

    // attach fake statistics
    statistics::attach_statistics_recursively(left);
    statistics::attach_statistics_recursively(right);

    auto join = make_node_join(nullptr, {"db", "result"});
    join->append_child(left);
    join->append_child(right);

    auto agg = make_node_aggregate(nullptr, {"db", "result"});
    agg->append_child(join);

    auto optimized = optimize(agg);

    REQUIRE(optimized->type() == node_type::aggregate_t);
    REQUIRE(optimized->children()[0]->type() == node_type::join_t);
    REQUIRE(optimized->estimated_rows() > 0);
}
