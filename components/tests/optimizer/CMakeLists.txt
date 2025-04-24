project(otterbrix_optimizer_tests)

set(SOURCE_FILES
    test_statistics.cpp
    test_estimator.cpp
    test_join_strategy.cpp
    test_optimizer.cpp
    test_rules.cpp
    test_composite_deep_plans.cpp
    test_constant_folding.cpp
    test_predicate_simplification.cpp
    test_memoization.cpp
    test_cbo_suite.cpp
)

add_executable(otterbrix_optimizer_tests ${SOURCE_FILES})

target_link_libraries(otterbrix_optimizer_tests
    PRIVATE
    otterbrix::logical_plan
    otterbrix::statistics
    otterbrix::optimizer
    otterbrix::expressions
    otterbrix::types
    Catch2::Catch2WithMain
)

target_include_directories(otterbrix_optimizer_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/components
)
