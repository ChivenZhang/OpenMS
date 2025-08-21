if(OPENMS_ENABLE_FILESERVER)
add_executable(FileServerService
    FileServerConfig.h
    FileServerService.cpp
    FileServerService.h
)
add_dependencies(FileServerService OpenMS)
target_include_directories(FileServerService PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(FileServerService PRIVATE OpenMS)
endif()