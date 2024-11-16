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
	Channel/HTTP/HTTPChannel.cpp
	Channel/HTTP/HTTPChannel.h
	Channel/HTTP/HTTPClientReactor.cpp
	Channel/HTTP/HTTPClientReactor.h
	Channel/HTTP/HTTPServerReactor.cpp
	Channel/HTTP/HTTPServerReactor.h
	Channel/IChannel.h
	Channel/IChannelAddress.h
	Channel/IChannelBuffer.h
	Channel/IChannelContext.h
	Channel/IChannelEvent.h
	Channel/IChannelHandler.h
	Channel/IChannelPipeline.h
	Channel/IChannelReactor.h
	Channel/IChannelWorker.h
	Channel/KCP/KCPChannel.cpp 
	Channel/KCP/KCPChannel.h 
	Channel/KCP/KCPClientReactor.cpp
	Channel/KCP/KCPClientReactor.h 
	Channel/KCP/KCPServerReactor.cpp 
	Channel/KCP/KCPServerReactor.h
	Channel/Private/Channel.cpp
	Channel/Private/Channel.h
	Channel/Private/ChannelAddress.cpp 
	Channel/Private/ChannelAddress.h 
	Channel/Private/ChannelBuffer.cpp
	Channel/Private/ChannelBuffer.h
	Channel/Private/ChannelContext.cpp
	Channel/Private/ChannelContext.h
	Channel/Private/ChannelEvent.h 
	Channel/Private/ChannelHandler.cpp
	Channel/Private/ChannelHandler.h
	Channel/Private/ChannelPipeline.cpp
	Channel/Private/ChannelPipeline.h
	Channel/Private/ChannelReactor.cpp
	Channel/Private/ChannelReactor.h
	Channel/Private/ChannelWorker.cpp
	Channel/Private/ChannelWorker.h
	Channel/TCP/TCPChannel.cpp 
	Channel/TCP/TCPChannel.h 
	Channel/TCP/TCPClientReactor.cpp 
	Channel/TCP/TCPClientReactor.h 
	Channel/TCP/TCPServerReactor.cpp 
	Channel/TCP/TCPServerReactor.h
	Channel/UDP/UDPChannel.cpp
	Channel/UDP/UDPChannel.h 
	Channel/UDP/UDPClientReactor.cpp 
	Channel/UDP/UDPClientReactor.h 
	Channel/UDP/UDPServerReactor.cpp 
	Channel/UDP/UDPServerReactor.h
	Channel/WS/WSChannel.cpp
	Channel/WS/WSChannel.h
	Channel/WS/WSClientReactor.cpp
	Channel/WS/WSClientReactor.h
	Channel/WS/WSServerReactor.cpp
	Channel/WS/WSServerReactor.h
	MS.h
	Remote/IRemoteFunction.h
	Service/IService.h
	Service/Private/Service.cpp
	Service/Private/Service.h
)
add_dependencies(OpenMS uv_a)

target_include_directories(OpenMS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PUBLIC ${CMAKE_SOURCE_DIR}/External/http-parser)
target_include_directories(OpenMS PUBLIC ${CMAKE_SOURCE_DIR}/External/libuv/include)
target_include_directories(OpenMS PUBLIC ${CMAKE_SOURCE_DIR}/External/nlohmann/single_include)
target_link_libraries(OpenMS uv_a)
