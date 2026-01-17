if(OPENMS_ENABLE_MYSQL)
add_executable(MySQLServer
        MySQLServer.cpp
        MySQLServer.h
)
add_dependencies(MySQLServer OpenMS)
target_include_directories(MySQLServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(MySQLServer PRIVATE OpenMS)
endif()

if(OPENMS_ENABLE_REDIS)
add_executable(RedisServer
        RedisServer.cpp
        RedisServer.h
)
add_dependencies(RedisServer OpenMS)
target_include_directories(RedisServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(RedisServer PRIVATE OpenMS)
endif()

#if(OPENMS_ENABLE_SQLITE)
#add_executable(SQLiteServer
#        SQLiteServer.cpp
#        SQLiteServer.h
#)
#add_dependencies(SQLiteServer OpenMS)
#target_include_directories(SQLiteServer PUBLIC ${OPENMS_SOURCE_DIR})
#target_link_libraries(SQLiteServer PRIVATE OpenMS)
#endif()