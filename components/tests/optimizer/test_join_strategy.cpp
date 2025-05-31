//
//  Проверяем выбор стратегии оптимизатора на синтетических планах
//
#include <catch2/catch.hpp>

#include <components/logical_plan/node_data.hpp>
#include <optimizer/cost/join_strategy.hpp>
#include <statistics/table_statistics.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::cost;
namespace cs = components::statistics;

// утилита: создаём Data-узел с заданным row_count
static node_ptr make_table(std::size_t rows,
                           std::pmr::memory_resource* res = nullptr)
{
    auto n   = make_node_data(res, {"db", "t"});
    auto st  = std::make_shared<cs::TableStatistics>(res);
    st->row_count = rows;
    n->set_statistics(st);
    return n;
}

TEST_CASE("CBO chooses Hash Join for сравнительно небольших входов")
{
    auto left  = make_table(10'000);
    auto right = make_table(2'000);

    REQUIRE(choose_join_strategy(left, right) == join_strategy_t::hash);
}

TEST_CASE("CBO chooses Nested-Loop when both inputs tiny")
{
    auto left  = make_table(50);
    auto right = make_table(40);

    REQUIRE(choose_join_strategy(left, right) == join_strategy_t::nested_loop);
}

TEST_CASE("CBO chooses Merge Join when объёмы сильно различаются")
{
    auto big     = make_table(1'000'000);
    auto small   = make_table(20'000);

    REQUIRE(choose_join_strategy(big, small) == join_strategy_t::merge);
}

TEST_CASE("CBO chooses Index Nested Loop when right < 20 rows и «есть индекс»")
{
    auto left  = make_table(100'000);
    auto right = make_table(15);

    // «Флажок» наличия индекса: обнуляем селективность
    right->statistics()->selectivity = 0.0;

    REQUIRE(choose_join_strategy(left, right) == join_strategy_t::index_nested_loop);
}

TEST_CASE("CBO chooses Grace-Hash when оба входа большие, не помещаются в RAM")
{
    auto left  = make_table(5'000'000);
    auto right = make_table(4'000'000);

    // Подразумевается, что оценщик заметит rows_out > threshold_memory
    REQUIRE(choose_join_strategy(left, right) == join_strategy_t::grace_hash);
}
