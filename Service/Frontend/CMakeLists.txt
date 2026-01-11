if(OPENMS_ENABLE_BUSINESS)
add_executable(FrontendClient
        FrontendClient.cpp
        FrontendClient.h
        ClientService.cpp
        ClientService.h
        PlayerService.cpp
        PlayerService.h
)
add_dependencies(FrontendClient OpenMS)
target_include_directories(FrontendClient PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(FrontendClient PRIVATE OpenMS)
endif()