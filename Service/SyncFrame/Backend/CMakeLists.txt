if(OPENMS_ENABLE_SYNC_FRAME)
add_executable(FrameBackend
        BackendServer.cpp
        BackendServer.h
        PlayerService.cpp
        PlayerService.h
        SpaceService.cpp
        SpaceService.h
)

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
target_link_libraries(FrameBackend PRIVATE gflags::gflags)

add_dependencies(FrameBackend OpenMS)
target_include_directories(FrameBackend PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(FrameBackend PRIVATE OpenMS)
endif()