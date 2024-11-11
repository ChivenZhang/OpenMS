cmake_minimum_required (VERSION 3.8)

add_definitions(
	#-DOPENMS_SHARED_LIBRARY
)

add_library(OpenMS STATIC
	IChannel.h
	IChannelEvent.h
	IChannelHandler.h
	IChannelPipeline.h
	IChannelReactor.h
	IChannelWorker.h
	IChannelContext.h
	IChannelBuffer.h
	IChannelAddress.h
	IChannelProtocol.h
	Private/Channel.h
	Private/Channel.cpp
	Private/ChannelProtocol.h
	Private/ChannelBuffer.h
	Private/ChannelBuffer.cpp
	Private/ChannelContext.h
	Private/ChannelContext.cpp
	Private/ChannelHandler.h
	Private/ChannelHandler.cpp
	Private/ChannelPipeline.h
	Private/ChannelPipeline.cpp
	Private/ChannelReactor.h
	Private/ChannelReactor.cpp
	Private/ChannelWorker.h
	Private/ChannelWorker.cpp
	IService.h
	MS.h
	Private/Service.cpp
	Private/Service.h

 "HTTP/HTTPReactor.h" "KCP/KCPReactor.h" "TCP/TCPReactor.h" "UDP/UDPReactor.h")
add_dependencies(OpenMS uv_a)

target_include_directories(OpenMS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PRIVATE ${CMAKE_SOURCE_DIR}/External/libuv/include)
target_link_libraries(OpenMS uv_a)