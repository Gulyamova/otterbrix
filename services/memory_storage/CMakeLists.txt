project(memory_storage)

set(${PROJECT_NAME}_SOURCES
    memory_storage.cpp
)

add_library(otterbrix_${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

add_library(otterbrix::${PROJECT_NAME} ALIAS otterbrix_${PROJECT_NAME})

set_property(TARGET otterbrix_${PROJECT_NAME} PROPERTY EXPORT_NAME ${PROJECT_NAME})

target_link_libraries(
    otterbrix_${PROJECT_NAME} PRIVATE
    otterbrix::collection
    otterbrix::disk
    otterbrix::locks
    otterbrix::cursor
    otterbrix::session
    spdlog::spdlog
    actor-zeta::actor-zeta
)

target_include_directories(otterbrix_${PROJECT_NAME} PUBLIC
)
