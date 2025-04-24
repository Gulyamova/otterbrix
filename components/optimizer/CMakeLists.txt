project(optimizer CXX)

set(SOURCE_${PROJECT_NAME}
        optimizer.cpp
        cost/estimator.cpp
        cost/join_strategy.cpp
        cost/table_cost_model.cpp
        rules/filter_pushdown.cpp
        rules/limit_pushdown.cpp
        rules/join_reordering.cpp
        utils/memo.cpp
)

add_library(otterbrix_${PROJECT_NAME}
        ${SOURCE_${PROJECT_NAME}}
)

add_library(otterbrix::${PROJECT_NAME} ALIAS otterbrix_${PROJECT_NAME})

set_property(TARGET otterbrix_${PROJECT_NAME} PROPERTY EXPORT_NAME ${PROJECT_NAME})

target_link_libraries(
        otterbrix_${PROJECT_NAME} PRIVATE
        otterbrix::logical_plan  
        otterbrix::statistics
)


target_include_directories(
        otterbrix_${PROJECT_NAME}
        PUBLIC
)

