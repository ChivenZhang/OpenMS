cmake_minimum_required (VERSION 3.8)

add_definitions(
	#-DOPENMS_SHARED_LIBRARY
)

include_directories(
	${CMAKE_SOURCE_DIR}/External/kcp
)

message(${CMAKE_SOURCE_DIR}/External/kcp)
add_library(OpenMS STATIC
	${CMAKE_SOURCE_DIR}/External/kcp/ikcp.c
	${CMAKE_SOURCE_DIR}/External/kcp/ikcp.h
	HTTP/HTTPReactor.h
	IChannel.h
	IChannelAddress.h
	IChannelBuffer.h
	IChannelContext.h
	IChannelEvent.h
	IChannelHandler.h
	IChannelPipeline.h
	IChannelReactor.h
	IChannelWorker.h
	IService.h
	KCP/KCPChannel.cpp 
	KCP/KCPChannel.h 
	KCP/KCPClientReactor.cpp
	KCP/KCPClientReactor.h 
	KCP/KCPServerReactor.cpp 
	KCP/KCPServerReactor.h
	MS.h
	Private/Channel.cpp
	Private/Channel.h
	Private/ChannelAddress.cpp 
	Private/ChannelAddress.h 
	Private/ChannelBuffer.cpp
	Private/ChannelBuffer.h
	Private/ChannelContext.cpp
	Private/ChannelContext.h
	Private/ChannelEvent.h 
	Private/ChannelHandler.cpp
	Private/ChannelHandler.h
	Private/ChannelPipeline.cpp
	Private/ChannelPipeline.h
	Private/ChannelReactor.cpp
	Private/ChannelReactor.h
	Private/ChannelWorker.cpp
	Private/ChannelWorker.h
	Private/Service.cpp
	Private/Service.h
	TCP/TCPChannel.cpp 
	TCP/TCPChannel.h 
	TCP/TCPClientReactor.cpp 
	TCP/TCPClientReactor.h 
	TCP/TCPServerReactor.cpp 
	TCP/TCPServerReactor.h
	UDP/UDPChannel.cpp
	UDP/UDPChannel.h 
	UDP/UDPClientReactor.cpp 
	UDP/UDPClientReactor.h 
	UDP/UDPServerReactor.cpp 
	UDP/UDPServerReactor.h
	WS/WSReactor.h
)
add_dependencies(OpenMS uv_a)

target_include_directories(OpenMS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PRIVATE ${CMAKE_SOURCE_DIR}/External/libuv/include)
target_link_libraries(OpenMS uv_a)