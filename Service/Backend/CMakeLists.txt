if(OPENMS_ENABLE_BACKEND)
add_executable(BackendServer
        BackendServer.cpp
        BackendServer.h
        PlayerService.cpp
        PlayerService.h
        SpaceService.cpp
        SpaceService.h
)

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
target_link_libraries(BackendServer PRIVATE gflags::gflags)

add_dependencies(BackendServer OpenMS)
target_include_directories(BackendServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(BackendServer PRIVATE OpenMS)
endif()