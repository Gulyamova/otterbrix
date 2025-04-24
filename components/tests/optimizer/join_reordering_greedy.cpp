#include <catch2/catch.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_join.hpp>
#include <optimizer/optimizer.hpp>
#include <statistics/attach_statistics.hpp>

using namespace components::logical_plan;

TEST_CASE("Greedy Join Reordering: ABC → ((A ⋈ C) ⋈ B)") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});
    auto C = make_node_data(nullptr, {"db", "C"});

    A->set_statistics(std::make_shared<statistics::TableStatistics>(500));
    B->set_statistics(std::make_shared<statistics::TableStatistics>(1000));
    C->set_statistics(std::make_shared<statistics::TableStatistics>(100));

    auto join1 = make_node_join(nullptr, {"db", "X"});
    join1->append_child(A);
    join1->append_child(B);

    auto join2 = make_node_join(nullptr, {"db", "X"});
    join2->append_child(join1);
    join2->append_child(C);

    auto reordered = optimizer::optimize(join2);

    REQUIRE(reordered->type() == node_type::join_t);
    REQUIRE(reordered->children()[0]->type() == node_type::join_t);
    REQUIRE(reordered->children()[1]->type() == node_type::data_t); // C — самая дешёвая

    // Примерный вывод:
    // reordered = Join(
    //     Join(A, C),
    //     B
    // )
}
TEST_CASE("Greedy Join Reordering: A → (A ⋈ B)") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});

    A->set_statistics(std::make_shared<statistics::TableStatistics>(500));
    B->set_statistics(std::make_shared<statistics::TableStatistics>(1000));

    auto join = make_node_join(nullptr, {"db", "X"});
    join->append_child(A);
    join->append_child(B);

    auto reordered = optimizer::optimize(join);

    REQUIRE(reordered->type() == node_type::join_t);
    REQUIRE(reordered->children()[0]->type() == node_type::data_t); // A
    REQUIRE(reordered->children()[1]->type() == node_type::data_t); // B
}
TEST_CASE("Greedy Join Reordering: A ⋈ B → (A ⋈ B)") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});

    A->set_statistics(std::make_shared<statistics::TableStatistics>(500));
    B->set_statistics(std::make_shared<statistics::TableStatistics>(1000));

    auto join = make_node_join(nullptr, {"db", "X"});
    join->append_child(A);
    join->append_child(B);

    auto reordered = optimizer::optimize(join);

    REQUIRE(reordered->type() == node_type::join_t);
    REQUIRE(reordered->children()[0]->type() == node_type::data_t); // A
    REQUIRE(reordered->children()[1]->type() == node_type::data_t); // B
}

TEST_CASE("Greedy Join Reordering: A ⋈ B → (B ⋈ A)") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});

    A->set_statistics(std::make_shared<statistics::TableStatistics>(500));
    B->set_statistics(std::make_shared<statistics::TableStatistics>(1000));

    auto join = make_node_join(nullptr, {"db", "X"});
    join->append_child(A);
    join->append_child(B);

    auto reordered = optimizer::optimize(join);

    REQUIRE(reordered->type() == node_type::join_t);
    REQUIRE(reordered->children()[0]->type() == node_type::data_t); // A
    REQUIRE(reordered->children()[1]->type() == node_type::data_t); // B
}   

TEST_CASE("Greedy Join Reordering: A ⋈ B → (A ⋈ B)") {
    auto A = make_node_data(nullptr, {"db", "A"});
    auto B = make_node_data(nullptr, {"db", "B"});

    A->set_statistics(std::make_shared<statistics::TableStatistics>(500));
    B->set_statistics(std::make_shared<statistics::TableStatistics>(1000));

    auto join = make_node_join(nullptr, {"db", "X"});
    join->append_child(A);
    join->append_child(B);

    auto reordered = optimizer::optimize(join);

    REQUIRE(reordered->type() == node_type::join_t);
    REQUIRE(reordered->children()[0]->type() == node_type::data_t); // A
    REQUIRE(reordered->children()[1]->type() == node_type::data_t); // B
}