
add_executable(RegistryService
	main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/RegistryService.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/RegistryService.h
)
add_dependencies(RegistryService OpenMS)

target_include_directories(RegistryService PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RegistryService PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(RegistryService OpenMS)