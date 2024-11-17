cmake_minimum_required (VERSION 3.8)

add_definitions(
	#-DOPENMS_SHARED_LIBRARY
)

include_directories(
	${CMAKE_SOURCE_DIR}/External/kcp
)

add_library(OpenMS STATIC
	${CMAKE_SOURCE_DIR}/External/kcp/ikcp.c
	${CMAKE_SOURCE_DIR}/External/kcp/ikcp.h
	${CMAKE_SOURCE_DIR}/External/http-parser/http_parser.c
	${CMAKE_SOURCE_DIR}/External/http-parser/http_parser.h
	Reactor/IChannel.h
	Reactor/IChannelAddress.h
	Reactor/IChannelBuffer.h
	Reactor/IChannelContext.h
	Reactor/IChannelEvent.h
	Reactor/IChannelHandler.h
	Reactor/IChannelPipeline.h
	Reactor/IChannelReactor.h
	Reactor/IChannelWorker.h
	Reactor/KCP/KCPChannel.cpp 
	Reactor/KCP/KCPChannel.h 
	Reactor/KCP/KCPClientReactor.cpp
	Reactor/KCP/KCPClientReactor.h 
	Reactor/KCP/KCPServerReactor.cpp 
	Reactor/KCP/KCPServerReactor.h
	Reactor/Private/Channel.cpp
	Reactor/Private/Channel.h
	Reactor/Private/ChannelAddress.cpp 
	Reactor/Private/ChannelAddress.h 
	Reactor/Private/ChannelBuffer.cpp
	Reactor/Private/ChannelBuffer.h
	Reactor/Private/ChannelContext.cpp
	Reactor/Private/ChannelContext.h
	Reactor/Private/ChannelEvent.h 
	Reactor/Private/ChannelHandler.cpp
	Reactor/Private/ChannelHandler.h
	Reactor/Private/ChannelPipeline.cpp
	Reactor/Private/ChannelPipeline.h
	Reactor/Private/ChannelReactor.cpp
	Reactor/Private/ChannelReactor.h
	Reactor/Private/ChannelWorker.cpp
	Reactor/Private/ChannelWorker.h
	Reactor/TCP/TCPChannel.cpp 
	Reactor/TCP/TCPChannel.h 
	Reactor/TCP/TCPClientReactor.cpp 
	Reactor/TCP/TCPClientReactor.h 
	Reactor/TCP/TCPServerReactor.cpp 
	Reactor/TCP/TCPServerReactor.h
	Reactor/UDP/UDPChannel.cpp
	Reactor/UDP/UDPChannel.h 
	Reactor/UDP/UDPClientReactor.cpp 
	Reactor/UDP/UDPClientReactor.h 
	Reactor/UDP/UDPServerReactor.cpp 
	Reactor/UDP/UDPServerReactor.h
	MS.h
	Remote/IRemoteFunction.h
	Service/IService.h
	Service/Private/Service.cpp
	Service/Private/Service.h
 "Endpoint/IEndpoint.h")
add_dependencies(OpenMS uv_a)

target_include_directories(OpenMS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PUBLIC ${CMAKE_SOURCE_DIR}/External/http-parser)
target_include_directories(OpenMS PUBLIC ${CMAKE_SOURCE_DIR}/External/libuv/include)
target_include_directories(OpenMS PUBLIC ${CMAKE_SOURCE_DIR}/External/nlohmann/single_include)
target_link_libraries(OpenMS uv_a)
