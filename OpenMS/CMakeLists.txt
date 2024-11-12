cmake_minimum_required (VERSION 3.8)

add_definitions(
	#-DOPENMS_SHARED_LIBRARY
)

add_library(OpenMS STATIC
	HTTP/HTTPReactor.h
	IChannel.h
	IChannelAddress.h
	IChannelBuffer.h
	IChannelContext.h
	IChannelEvent.h
	IChannelHandler.h
	IChannelPipeline.h
	IChannelProtocol.h
	IChannelReactor.h
	IChannelWorker.h
	IService.h
	KCP/KCPReactor.h
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
	Private/ChannelProtocol.h
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
	UDP/UDPServerReactor.cpp 
	UDP/UDPServerReactor.h
	WS/WSReactor.h
 "UDP/UDPClientReactor.h" "UDP/UDPClientReactor.cpp")
add_dependencies(OpenMS uv_a)

target_include_directories(OpenMS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PRIVATE ${CMAKE_SOURCE_DIR}/External/libuv/include)
target_link_libraries(OpenMS uv_a)