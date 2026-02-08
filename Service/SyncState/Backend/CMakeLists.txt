if(OPENMS_ENABLE_SYNC_STATE)
add_executable(StateBackend
        BackendServer.cpp
        BackendServer.h
        PlayerService.cpp
        PlayerService.h
        SpaceService.cpp
        SpaceService.h
)

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
target_link_libraries(StateBackend PRIVATE gflags::gflags)

add_dependencies(StateBackend OpenMS)
target_include_directories(StateBackend PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(StateBackend PRIVATE OpenMS)
endif()