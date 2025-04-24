#include <catch2/catch.hpp>
#include <optimizer/cost/estimator.hpp>
#include <components/logical_plan/node_data.hpp>
#include <statistics/table_statistics.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::cost;

TEST_CASE("Estimator uses statistics row_count") {
    auto node = make_node_data(nullptr, {"db", "test"});
    node->set_statistics(std::make_shared<statistics::TableStatistics>(5000));

    estimate_node_output_rows(node);

    REQUIRE(node->estimated_rows() == 5000);
}
