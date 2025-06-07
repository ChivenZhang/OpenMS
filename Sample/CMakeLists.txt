add_executable(MasterDemo
		MasterDemo.cpp
		MasterDemo.h
)
add_dependencies(MasterDemo OpenMS)
target_include_directories(MasterDemo PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(MasterDemo PRIVATE OpenMS)


add_executable(ClusterDemo1
		ClusterDemo1.cpp
		ClusterDemo1.h
)
add_dependencies(ClusterDemo1 OpenMS)
target_include_directories(ClusterDemo1 PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(ClusterDemo1 PRIVATE OpenMS)


add_executable(ClusterDemo2
		ClusterDemo2.cpp
		ClusterDemo2.h
)
add_dependencies(ClusterDemo2 OpenMS)
target_include_directories(ClusterDemo2 PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(ClusterDemo2 PRIVATE OpenMS)