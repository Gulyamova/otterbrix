#include <catch2/catch_test_macros.hpp>
#include <optimizer/cost/join_strategy.hpp>
#include <components/logical_plan/node.hpp>
#include <components/logical_plan/node_data.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::cost;

// Созднаие узлов с заданным количеством строк
node_ptr make_mock_node(size_t rows) {
    auto n = make_node_data(nullptr, {"db", "collection"});
    n->set_estimated_rows(rows);
    return n;
}

TEST_CASE("choose_join_strategy selects appropriate strategy") {

    SECTION("Hash join preferred for large tables") {
        auto left = make_mock_node(10'000);
        auto right = make_mock_node(8'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::hash);
    }

    SECTION("Nested Loop preferred for small tables") {
        auto left = make_mock_node(30);
        auto right = make_mock_node(40);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::nested_loop);
    }

    SECTION("Merge join selected for sorted medium tables") {
        auto left = make_mock_node(1'000);
        auto right = make_mock_node(1'200);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::merge);
    }

    SECTION("Grace hash is fallback for very large and memory-limited tables") {
        auto left = make_mock_node(100'000);
        auto right = make_mock_node(100'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::grace_hash);
    }

    SECTION("Index Nested Loop returns infinity cost when no index") {
        auto left = make_mock_node(500);
        auto right = make_mock_node(1'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy != join_strategy_t::index_nested_loop); // пока не поддержан
    }

    SECTION("Identical tables prefer hash join") {
        auto left = make_mock_node(3'000);
        auto right = make_mock_node(3'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::hash);
    }

    SECTION("Highly unbalanced tables still prefer hash") {
        auto left = make_mock_node(10'000);
        auto right = make_mock_node(20);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::hash);
    }

    SECTION("Tiny table and large table prefer nested loop") {
        auto left = make_mock_node(5);
        auto right = make_mock_node(10'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::nested_loop);
    }

    SECTION("Grace hash dominates when memory spill expected") {
        auto left = make_mock_node(200'000);
        auto right = make_mock_node(300'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::grace_hash);
    }

    SECTION("Merge join is not chosen without sorted input (yet)") {
        auto left = make_mock_node(5'000);
        auto right = make_mock_node(6'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy != join_strategy_t::merge);
    }

    SECTION("Avoid index nested loop without index support") {
        auto left = make_mock_node(100);
        auto right = make_mock_node(100'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy != join_strategy_t::index_nested_loop);
    }

    SECTION("Large identical tables prefer hash join again") {
        auto left = make_mock_node(100'000);
        auto right = make_mock_node(100'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::hash);
    }

    SECTION("Low-cost joins correctly chosen by heuristic") {
        auto left = make_mock_node(1);
        auto right = make_mock_node(1);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::nested_loop);
    }

    SECTION("One empty table results in zero cost") {
        auto left = make_mock_node(0);
        auto right = make_mock_node(1'000);
        auto strategy = choose_join_strategy(left, right);
        REQUIRE(strategy == join_strategy_t::nested_loop);
    }

    SECTION("Symmetric inputs yield same result regardless of order") {
        auto A = make_mock_node(5000);
        auto B = make_mock_node(7000);
        auto AB = choose_join_strategy(A, B);
        auto BA = choose_join_strategy(B, A);
        REQUIRE(AB == BA);
    }
}
