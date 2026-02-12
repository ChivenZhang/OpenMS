if(OPENMS_ENABLE_SYNC_STATE)
project(StateBackend)
add_library(${PROJECT_NAME}SDK STATIC
        BackendServer.cpp
        BackendServer.h
        PlayerService.cpp
        PlayerService.h
        SpaceService.cpp
        SpaceService.h
)

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}SDK PUBLIC gflags::gflags)

add_dependencies(${PROJECT_NAME}SDK OpenMS)
target_include_directories(${PROJECT_NAME}SDK PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}SDK PUBLIC OpenMS)

add_executable(${PROJECT_NAME}
        StateBackend.cpp
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}SDK)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}SDK)
endif()