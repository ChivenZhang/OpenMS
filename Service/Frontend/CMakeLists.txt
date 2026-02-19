project(FrontendClient)

add_executable(${PROJECT_NAME}
        MyFrontend.cpp
)
add_dependencies(${PROJECT_NAME} StateFrontendSDK)
target_link_libraries(${PROJECT_NAME} PRIVATE StateFrontendSDK)