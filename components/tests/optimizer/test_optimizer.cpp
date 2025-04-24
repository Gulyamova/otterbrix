#include <catch2/catch.hpp>
#include <optimizer/optimizer.hpp>
#include <components/logical_plan/node_data.hpp>
#include <statistics/attach_statistics.hpp>

using namespace components::logical_plan;
using namespace optimizer;

TEST_CASE("Optimizer transforms node and sets estimates") {
    auto node = make_node_data(nullptr, {"db", "coll"});
    statistics::attach_statistics_recursively(node);

    auto result = optimize(node);

    REQUIRE(result != nullptr);
    REQUIRE(result->type() == node_type::data_t);
    REQUIRE(result->estimated_rows() >= 100);
}
