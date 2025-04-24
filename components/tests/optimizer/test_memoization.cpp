#include <catch2/catch.hpp>
#include <components/logical_plan/node_data.hpp>
#include <optimizer/utils/memo.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::utils;

TEST_CASE("Memo avoids duplicating subtree cost") {
    auto a = make_node_data(nullptr, {"db", "A"});
    auto b = make_node_data(nullptr, {"db", "B"});

    auto join1 = make_node_join(nullptr, {"db", "R"});
    join1->append_child(a);
    join1->append_child(b);

    auto join2 = make_node_join(nullptr, {"db", "R"});
    join2->append_child(a);  // reusing `a`
    join2->append_child(b);  // reusing `b`

    Memo memo;
    memo.insert(join1, 1.0);
    bool reused = memo.contains(join2);

    REQUIRE(reused);  // идентичные планы по структуре
}
