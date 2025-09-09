if(OPENMS_ENABLE_WEBSERVER)
add_executable(WebServerService
        WebServerConfig.h
        WebServerService.cpp
        WebServerService.h
)
add_dependencies(WebServerService OpenMS)
target_include_directories(WebServerService PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(WebServerService PRIVATE OpenMS)
endif()