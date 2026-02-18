project(BackendServer)

add_executable(${PROJECT_NAME}
        StateBackend.cpp
)
add_dependencies(${PROJECT_NAME} StateBackendSDK)
target_link_libraries(${PROJECT_NAME} PRIVATE StateBackendSDK)