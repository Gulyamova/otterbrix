#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <optimizer/rules/join_reorder.hpp>
#include <optimizer/cost/estimator.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_data.hpp>

using namespace components::optimizer::rules;
using namespace components::logical_plan;
using namespace cost;

TEST_CASE("JoinReorderRule: reorder joins based on estimated cost") {
    JoinReorderRule rule;

    auto make_scan = [](std::string name, double cost) {
        auto scan = make_node_data(nullptr, { "db", name });
        set_fake_cost(scan, cost);
        return scan;
    };

    auto join_names = [](const node_ptr& node) {
        std::vector<std::string> names;
        std::function<void(const node_ptr&)> collect = [&](const node_ptr& n) {
            if (n->type() == node_type::data_t) {
                names.push_back(n->collection_full_name().second);
            } else {
                for (const auto& c : n->children()) collect(c);
            }
        };
        collect(node);
        return names;
    };

    SECTION("2-way join reordered if right cheaper") {
        auto left = make_scan("a", 100);
        auto right = make_scan("b", 10);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(left);
        join->append_child(right);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());

        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "b", "a" });
    }

    SECTION("2-way join remains if left cheaper") {
        auto left = make_scan("a", 10);
        auto right = make_scan("b", 100);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(left);
        join->append_child(right);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());

        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "a", "b" });
    }

    SECTION("3-way join: reordered from middle cheapest") {
        auto a = make_scan("a", 100);
        auto b = make_scan("b", 5);
        auto c = make_scan("c", 50);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(a);
        join->append_child(b);
        join->append_child(c);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());

        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "b", "c", "a" });
    }

    SECTION("3-way join: reordered from right cheapest") {
        auto a = make_scan("a", 100);
        auto b = make_scan("b", 80);
        auto c = make_scan("c", 5);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(a);
        join->append_child(b);
        join->append_child(c);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());

        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "c", "b", "a" });
    }

    SECTION("3-way join: no reorder needed") {
        auto a = make_scan("a", 1);
        auto b = make_scan("b", 2);
        auto c = make_scan("c", 3);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(a);
        join->append_child(b);
        join->append_child(c);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());
        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "a", "b", "c" });
    }

    SECTION("Single input: no reorder") {
        auto scan = make_scan("only", 10);
        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(scan);

        auto result = rule.apply(join);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Empty input: no reorder") {
        auto join = make_node_join(nullptr, { "db", "joined" });
        auto result = rule.apply(join);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Nested join trees handled") {
        auto a = make_scan("a", 50);
        auto b = make_scan("b", 10);
        auto c = make_scan("c", 5);

        auto inner = make_node_join(nullptr, { "db", "inner" });
        inner->append_child(a);
        inner->append_child(b);

        auto outer = make_node_join(nullptr, { "db", "outer" });
        outer->append_child(inner);
        outer->append_child(c);

        auto result = rule.apply(outer);
        REQUIRE(result.has_value());

        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "c", "b", "a" });
    }

    SECTION("Recurse into children") {
        auto a = make_scan("a", 50);
        auto b = make_scan("b", 10);
        auto c = make_scan("c", 5);

        auto inner = make_node_join(nullptr, { "db", "inner" });
        inner->append_child(a);
        inner->append_child(b);

        auto outer = make_node_join(nullptr, { "db", "outer" });
        outer->append_child(inner);
        outer->append_child(c);

        auto result = rule.apply(outer);
        REQUIRE(result.has_value());

        auto order = join_names(*result);
        REQUIRE(order[0] == "c"); // Самый дешёвый должен быть первым
    }

    SECTION("4-way join: mixed cost") {
        auto a = make_scan("a", 30);
        auto b = make_scan("b", 20);
        auto c = make_scan("c", 10);
        auto d = make_scan("d", 1);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(a);
        join->append_child(b);
        join->append_child(c);
        join->append_child(d);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());
        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "d", "c", "b", "a" });
    }

    SECTION("Does not change non-join node") {
        auto scan = make_scan("a", 10);
        auto fake = make_node_data(nullptr, { "db", "fake" });
        fake->append_child(scan);

        auto result = rule.apply(fake);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Cost equals: maintains original order") {
        auto a = make_scan("a", 10);
        auto b = make_scan("b", 10);
        auto c = make_scan("c", 10);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(a);
        join->append_child(b);
        join->append_child(c);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());
        auto order = join_names(*result);
        REQUIRE(order.size() == 3);
    }

    SECTION("Join with reused child not reordered") {
        auto shared = make_scan("a", 1);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(shared);
        join->append_child(shared); // тот же узел дважды

        auto result = rule.apply(join);
        REQUIRE(result.has_value());
        auto order = join_names(*result);
        REQUIRE(order[0] == "a");
        REQUIRE(order[1] == "a");
    }

    SECTION("Handles already ordered plan gracefully") {
        auto a = make_scan("a", 1);
        auto b = make_scan("b", 2);
        auto c = make_scan("c", 3);

        auto ab = make_node_join(nullptr, { "db", "j1" });
        ab->append_child(a);
        ab->append_child(b);

        auto abc = make_node_join(nullptr, { "db", "j2" });
        abc->append_child(ab);
        abc->append_child(c);

        auto result = rule.apply(abc);
        REQUIRE(result.has_value());
        auto order = join_names(*result);
        REQUIRE(order == std::vector<std::string>{ "a", "b", "c" });
    }

    SECTION("Join cost is re-evaluated at each step") {
        auto a = make_scan("a", 100);
        auto b = make_scan("b", 10);
        auto c = make_scan("c", 1);

        auto join = make_node_join(nullptr, { "db", "joined" });
        join->append_child(a);
        join->append_child(b);
        join->append_child(c);

        auto result = rule.apply(join);
        REQUIRE(result.has_value());
        auto order = join_names(*result);
        REQUIRE(order[0] == "c");
    }
}
