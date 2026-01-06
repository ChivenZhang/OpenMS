if(OPENMS_ENABLE_DAEMON)
add_executable(DaemonServer
        DaemonServer.cpp
        DaemonServer.h
)
add_dependencies(DaemonServer OpenMS)
target_include_directories(DaemonServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(DaemonServer PRIVATE OpenMS)
endif()