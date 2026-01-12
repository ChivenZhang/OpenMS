if(OPENMS_ENABLE_BUSINESS)
add_executable(BusinessServer
        BusinessServer.cpp
        BusinessServer.h
        ServerService.cpp
        ServerService.h
        LogicService.cpp
        LogicService.h
        LoginService.cpp
        LoginService.h
)
add_dependencies(BusinessServer OpenMS)
target_include_directories(BusinessServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(BusinessServer PRIVATE OpenMS)
endif()