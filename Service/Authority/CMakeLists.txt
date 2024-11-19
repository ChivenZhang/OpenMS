
add_executable(AuthorityService
	AuthorityConfig.h
	AuthorityServer.cpp
	AuthorityServer.h
	AuthorityService.cpp
	AuthorityService.h
)
add_dependencies(AuthorityService OpenMS)

target_include_directories(AuthorityService PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(AuthorityService PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(AuthorityService OpenMS)

add_custom_command(
    TARGET AuthorityService POST_BUILD  
    COMMAND "${CMAKE_SOURCE_DIR}/INSTALL.bat"
    COMMENT "Copying dll to output directory"
)