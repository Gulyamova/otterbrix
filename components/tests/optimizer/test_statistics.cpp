#include <catch2/catch.hpp>
#include <statistics/attach_statistics.hpp>
#include <components/logical_plan/node_data.hpp>

using namespace components::logical_plan;
using namespace statistics;

TEST_CASE("Statistics are attached to logical plan node") {
    auto node = make_node_data(nullptr, {"db", "coll"});
    attach_statistics_recursively(node);

    REQUIRE(node->statistics() != nullptr);
    REQUIRE(std::static_pointer_cast<TableStatistics>(node->statistics())->row_count > 0);
}
