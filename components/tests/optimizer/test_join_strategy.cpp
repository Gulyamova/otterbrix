#include <catch2/catch.hpp>
#include <optimizer/cost/join_strategy.hpp>
#include <components/logical_plan/node_data.hpp>
#include <statistics/table_statistics.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::cost;

TEST_CASE("Join strategy selects hash join for small right") {
    auto left = make_node_data(nullptr, {"db", "l"});
    auto right = make_node_data(nullptr, {"db", "r"});

    left->set_statistics(std::make_shared<statistics::TableStatistics>(5000));
    right->set_statistics(std::make_shared<statistics::TableStatistics>(100));

    auto strategy = choose_join_strategy(left, right);
    REQUIRE(strategy == join_strategy_t::hash);
}
