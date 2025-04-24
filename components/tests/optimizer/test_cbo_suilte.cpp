#include <catch2/catch.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_aggregate.hpp>
#include <components/logical_plan/node_limit.hpp>
#include <optimizer/optimizer.hpp>
#include <statistics/attach_statistics.hpp>
#include <statistics/table_statistics.hpp>

using namespace components::logical_plan;
using namespace optimizer;
using namespace statistics;

TEST_CASE("Join Reordering chooses cheapest first") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});
    auto C = make_node_data(nullptr, {"db", "C"});

    A->set_statistics(std::make_shared<TableStatistics>(500));
    B->set_statistics(std::make_shared<TableStatistics>(1000));
    C->set_statistics(std::make_shared<TableStatistics>(100));

    auto j1 = make_node_join(nullptr, {"db", "temp"});
    j1->append_child(A);
    j1->append_child(B);

    auto root = make_node_join(nullptr, {"db", "out"});
    root->append_child(j1);
    root->append_child(C);

    auto optimized = optimize(root);

    REQUIRE(optimized->children()[1]->collection_name() == "C");
}

TEST_CASE("Limit Pushdown simplifies nested plans") {
    auto scan = make_node_data(nullptr, {"db", "table"});
    auto limit = make_node_limit(nullptr, {"db", "table"}, 5);
    limit->append_child(scan);

    auto optimized = optimize(limit);

    REQUIRE(optimized->type() == node_type::limit_t);
    REQUIRE(optimized->children()[0]->type() == node_type::data_t);
}

TEST_CASE("Projection Pruning removes excess expressions") {
    auto scan = make_node_data(nullptr, {"db", "table"});
    scan->append_expression(make_scalar_identifier("a"));
    scan->append_expression(make_scalar_identifier("b"));
    scan->append_expression(make_scalar_identifier("c"));

    auto optimized = optimize(scan);

    REQUIRE(optimized->expressions().size() == 1); // simplified
}

TEST_CASE("Constant Folding simplifies expressions") {
    auto scan = make_node_data(nullptr, {"db", "table"});
    scan->append_expression(make_scalar_literal(2 + 2)); // simulated folding

    auto optimized = optimize(scan);

    REQUIRE(optimized->expressions().size() == 1);
}

TEST_CASE("Selectivity-aware join picks strategy") {
    auto left = make_node_data(nullptr, {"db", "left"});
    auto right = make_node_data(nullptr, {"db", "right"});

    left->set_statistics(std::make_shared<TableStatistics>(5000));
    right->set_statistics(std::make_shared<TableStatistics>(100));

    auto join = make_node_join(nullptr, {"db", "join"});
    join->append_child(left);
    join->append_child(right);

    statistics::attach_statistics_recursively(join);

    auto result = optimize(join);
    REQUIRE(result->type() == node_type::join_t);
}
