if(OPENMS_ENABLE_BUSINESS)
add_executable(BusinessServer
        BusinessServer.cpp
        BusinessServer.h
        PlayerService.cpp
        PlayerService.h
        ServerService.cpp
        ServerService.h
)
add_dependencies(BusinessServer OpenMS)
target_include_directories(BusinessServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(BusinessServer PRIVATE OpenMS)
endif()