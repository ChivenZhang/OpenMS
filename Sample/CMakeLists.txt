
add_executable(MasterDemo
		MasterDemo.cpp
		MasterDemo.h
)
add_dependencies(MasterDemo OpenMS)
target_include_directories(MasterDemo PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(MasterDemo PRIVATE OpenMS)


add_executable(ClusterDemo
		ClusterDemo.cpp
		ClusterDemo.h
)
add_dependencies(ClusterDemo OpenMS)
target_include_directories(ClusterDemo PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(ClusterDemo PRIVATE OpenMS)