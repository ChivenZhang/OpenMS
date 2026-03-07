if(OPENMS_ENABLE_SYNC_STATE)
project(StateFrontendSDK)
add_library(${PROJECT_NAME} STATIC
        FrontendClient.cpp
        FrontendClient.h
        PlayerService.cpp
        PlayerService.h
        SpaceService.cpp
        SpaceService.h
)

add_dependencies(${PROJECT_NAME} OpenMS)
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC OpenMS)
endif()