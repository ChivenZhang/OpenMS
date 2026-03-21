if(OPENMS_ENABLE_SYNC_FRAME)
project(FrameBackendSDK)
add_executable(${PROJECT_NAME}
        BackendServer.cpp
        BackendServer.h
        PlayerService.cpp
        PlayerService.h
        SpaceService.cpp
        SpaceService.h
)

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE gflags::gflags)

add_dependencies(${PROJECT_NAME} OpenMS)
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenMS)
endif()