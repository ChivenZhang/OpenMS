if(OPENMS_ENABLE_MYSQL)
add_executable(MySQLService
    MySQLConfig.h
    MySQLService.cpp
    MySQLService.h
)
add_dependencies(MySQLService OpenMS)
target_include_directories(MySQLService PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(MySQLService PRIVATE OpenMS)
endif()

if(OPENMS_ENABLE_REDIS)
add_executable(RedisService
    RedisConfig.h
    RedisService.cpp
    RedisService.h
)
add_dependencies(RedisService OpenMS)
target_include_directories(RedisService PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(RedisService PRIVATE OpenMS)
endif()