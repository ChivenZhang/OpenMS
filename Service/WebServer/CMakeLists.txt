if(OPENMS_ENABLE_WEBSERVER)
add_executable(WebServer
        WebConfig.h
        WebServer.cpp
        WebServer.h
)
add_dependencies(WebServer OpenMS)
target_include_directories(WebServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(WebServer PRIVATE OpenMS)
endif()