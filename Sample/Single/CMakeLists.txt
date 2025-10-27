add_executable(KCPDemo
        KCPDemo.cpp
)
add_dependencies(KCPDemo OpenMS)
target_include_directories(KCPDemo PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(KCPDemo PRIVATE OpenMS)

add_executable(RPCDemo
        RPCDemo.cpp
)
add_dependencies(RPCDemo OpenMS)
target_include_directories(RPCDemo PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(RPCDemo PRIVATE OpenMS)

add_executable(TCPDemo
        TCPDemo.cpp
)
add_dependencies(TCPDemo OpenMS)
target_include_directories(TCPDemo PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(TCPDemo PRIVATE OpenMS)

add_executable(UDPDemo
        UDPDemo.cpp
)
add_dependencies(UDPDemo OpenMS)
target_include_directories(UDPDemo PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(UDPDemo PRIVATE OpenMS)