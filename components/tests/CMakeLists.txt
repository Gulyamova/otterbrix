project(test_generaty)

list(APPEND ${PROJECT_NAME}_SOURCES
        generaty.cpp
)

add_library(otterbrix_${PROJECT_NAME}
        ${${PROJECT_NAME}_SOURCES}
)

add_library(otterbrix::${PROJECT_NAME} ALIAS otterbrix_${PROJECT_NAME})

set_property(TARGET otterbrix_${PROJECT_NAME} PROPERTY EXPORT_NAME ${PROJECT_NAME})

target_link_libraries(
        otterbrix_${PROJECT_NAME} PRIVATE absl::numeric absl::int128 Boost::json
)

add_executable(test_optimizer test_optimizer.cpp)

target_include_directories(test_optimizer
    PRIVATE
        ${CMAKE_SOURCE_DIR}/..
)

target_link_libraries(test_optimizer
    PRIVATE
        otterbrix_logical_plan
        otterbrix_optimizer
        otterbrix_statistics
        otterbrix_explain
)
