if(OPENMS_ENABLE_SYNC_STATE)
project(StateFrontend)
add_library(${PROJECT_NAME}SDK STATIC
        FrontendClient.cpp
        FrontendClient.h
        ClientService.cpp
        ClientService.h
        PlayerService.cpp
        PlayerService.h
)

add_dependencies(${PROJECT_NAME}SDK OpenMS)
target_include_directories(${PROJECT_NAME}SDK PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}SDK PUBLIC OpenMS)

add_executable(${PROJECT_NAME}
        StateFrontend.cpp
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}SDK)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}SDK)
endif()