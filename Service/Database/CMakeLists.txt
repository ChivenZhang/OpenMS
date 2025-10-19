if(OPENMS_ENABLE_MYSQL)
add_executable(MySQLServer
        MySQLConfig.h
        MySQLServer.cpp
        MySQLServer.h
)
add_dependencies(MySQLServer OpenMS)
target_include_directories(MySQLServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(MySQLServer PRIVATE OpenMS)
endif()

if(OPENMS_ENABLE_REDIS)
add_executable(RedisServer
        RedisConfig.h
        RedisServer.cpp
        RedisServer.h
)
add_dependencies(RedisServer OpenMS)
target_include_directories(RedisServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(RedisServer PRIVATE OpenMS)
endif()