cmake_minimum_required (VERSION 3.8)

add_definitions(
	#-DOPENMS_SHARED_LIBRARY
)

include_directories(
	${OPENMS_SOURCE_DIR}/External/kcp
)

add_library(OpenMS STATIC
	${OPENMS_SOURCE_DIR}/External/kcp/ikcp.c
	${OPENMS_SOURCE_DIR}/External/kcp/ikcp.h
	${OPENMS_SOURCE_DIR}/External/http-parser/http_parser.c
	${OPENMS_SOURCE_DIR}/External/http-parser/http_parser.h
	Endpoint/HTTP/HTTPClient.cpp
	Endpoint/HTTP/HTTPClient.h
	Endpoint/HTTP/HTTPServer.cpp
	Endpoint/HTTP/HTTPServer.h 
	Endpoint/IEndpoint.h 
	Endpoint/KCP/KCPClient.cpp 
	Endpoint/KCP/KCPClient.h 
	Endpoint/KCP/KCPServer.cpp 
	Endpoint/KCP/KCPServer.h 
	Endpoint/Private/Endpoint.cpp
	Endpoint/Private/Endpoint.h
	Endpoint/RPC/RPCClient.cpp
	Endpoint/RPC/RPCClient.h
	Endpoint/RPC/RPCProtocol.h
	Endpoint/RPC/RPCServer.cpp
	Endpoint/RPC/RPCServer.h
	Endpoint/TCP/TCPClient.cpp 
	Endpoint/TCP/TCPClient.h 
	Endpoint/TCP/TCPServer.cpp 
	Endpoint/TCP/TCPServer.h
	Endpoint/UDP/UDPClient.cpp 
	Endpoint/UDP/UDPClient.h 
	Endpoint/UDP/UDPServer.cpp
	Endpoint/UDP/UDPServer.h 
	Endpoint/WS/WSClient.cpp 
	Endpoint/WS/WSClient.h 
	Endpoint/WS/WSServer.cpp 
	Endpoint/WS/WSServer.h 
	MS.h
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
	Reactor/Private/ChannelEvent.cpp
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
	Service/IProperty.h
	Service/IService.h
	Service/Private/Property.cpp
	Service/Private/Property.h
	Service/Private/Service.cpp
	Service/Private/Service.h
	Toolkit/Timer.cpp
	Toolkit/Timer.h
	Handler/AES/AESHandler.cpp
	Handler/AES/AESHandler.h
		Mailbox/IMailbox.h
		Mailbox/IMailContext.h
		Mailbox/Private/MailContext.cpp
		Mailbox/Private/MailContext.h
		Mailbox/Private/MailDeliver.cpp
		Mailbox/Private/MailDeliver.h
		Mailbox/Private/Mailbox.cpp
		Mailbox/Private/Mailbox.h
)
add_dependencies(OpenMS uv_a mbedtls)

target_include_directories(OpenMS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PUBLIC ${OPENMS_SOURCE_DIR}/External/iocpp)
target_include_directories(OpenMS PUBLIC ${OPENMS_SOURCE_DIR}/External/http-parser)
target_include_directories(OpenMS PUBLIC ${OPENMS_SOURCE_DIR}/External/libuv/include)
target_include_directories(OpenMS PUBLIC ${OPENMS_SOURCE_DIR}/External/mbedtls/include)
target_include_directories(OpenMS PUBLIC ${OPENMS_SOURCE_DIR}/External/nlohmann/single_include)
target_link_libraries(OpenMS uv_a iocpp mbedcrypto mbedtls mbedx509)
