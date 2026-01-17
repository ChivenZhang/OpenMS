if(OPENMS_ENABLE_STORAGE)
add_executable(StorageServer
        StorageServer.cpp
        StorageServer.h
        UserService.cpp
        UserService.h
)
add_dependencies(StorageServer OpenMS)
target_include_directories(StorageServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(StorageServer PRIVATE OpenMS)
endif()