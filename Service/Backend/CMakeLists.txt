if(OPENMS_ENABLE_BACKEND)
add_executable(BackendServer
        BackendServer.cpp
        BackendServer.h
)
add_dependencies(BackendServer OpenMS)
target_include_directories(BackendServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(BackendServer PRIVATE OpenMS)
endif()