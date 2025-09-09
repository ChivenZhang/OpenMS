/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "WebServerService.h"
#include <regex>
#include <fstream>
#include <filesystem>
#include <cpptrace.hpp>

class WebServerErrorHandler : public HTTPServerResponseHandler
{
public:
	explicit WebServerErrorHandler(MSRaw<WebServerService> server)
		:
		m_Server(server)
	{
	}

protected:
	bool handle(MSRaw<IChannelContext> context, request_t const& request, response_t& response) override
	{
		if (response.Code)
		{
			// Handle error code
			if(response.Code != HTTP_STATUS_OK)
			{
				try
				{
					auto result = m_Server->m_ErrorPages.find(std::to_string(response.Code));
					if(result != m_Server->m_ErrorPages.end())
					{
						m_Server->forward(result->second, response);
					}
				}
				catch (MSError const& ex)
				{
					cpptrace::logic_error error(ex.what());
					MS_ERROR("%s", error.what());
				}
				catch (...)
				{
					cpptrace::logic_error error("unhandled exception");
					MS_ERROR("%s", error.what());
				}
			}
		}
		return true;
	}

protected:
	MSRaw<WebServerService> m_Server;
};

MSString WebServerService::identity() const
{
	return "webserver";
}

void WebServerService::onInit()
{
	ClusterService::onInit();

	HTTPServer::startup();

	// URL alias

	for (auto& pattern : m_StaticAlias)
	{
		auto& staticName = pattern.first;
		auto& staticPaths = pattern.second;
		bind_get(staticName + ".*", [=, this](request_t const& request, response_t& response)
		{
			MS_INFO("alias %s", request.Url.c_str());
			try
			{
				std::regex regex(staticName + "(.*)");
				std::smatch match;
				if (std::regex_search(request.Url, match, regex))
				{
					auto filePath = match[1].str();
					for (auto& staticPath : staticPaths)
					{
						auto fullPath = std::filesystem::path(staticPath + "/" + filePath);
						if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
						{

							auto fileSize = std::filesystem::file_size(fullPath);
							if(m_MaxBodySize && m_MaxBodySize < fileSize)
							{
								response.Code = HTTP_STATUS_PAYLOAD_TOO_LARGE;
								response.Body = "413 Payload Too Large";
								return;
							}
							std::ifstream file(fullPath, std::ios::in | std::ios::binary);
							if (file.is_open())
							{
								response.Code = HTTP_STATUS_OK;
								response.Body.resize(fileSize);
								response.Header["Content-Length"] = std::to_string(file.read(response.Body.data(), (int64_t)response.Body.size()).gcount());
								file.close();

								auto fileType = std::filesystem::path(request.Url).extension().generic_string();
								auto result = OPENMS_MIME_TYPE.find(fileType);
								if (result == OPENMS_MIME_TYPE.end()) response.Header["Content-Type"] = "text/plain";
								else response.Header["Content-Type"] = result->second;
								return;
							}
						}
					}
				}
			}
			catch (MSError& ex)
			{
				MS_ERROR("%s", ex.what());
				return;
			}
			response.Code = HTTP_STATUS_NOT_FOUND;
			response.Body = "404 Not Found";
		});
	}

	// URL roots

	for (auto& pattern : m_StaticRoots)
	{
		auto& staticName = pattern.first;
		auto& staticPath = pattern.second;
		bind_get(staticName + ".*", [=, this](request_t const& request, response_t& response)
		{
			MS_INFO("root %s", request.Url.c_str());

			auto filePath = request.Url;
			auto fullPath = std::filesystem::path(staticPath + filePath);
			if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
			{
				auto fileSize = std::filesystem::file_size(fullPath);
				if(m_MaxBodySize && m_MaxBodySize < fileSize)
				{
					response.Code = HTTP_STATUS_PAYLOAD_TOO_LARGE;
					response.Body = "413 Payload Too Large";
					return;
				}
				std::ifstream file(fullPath, std::ios::in | std::ios::binary);
				if (file.is_open())
				{
					response.Code = HTTP_STATUS_OK;
					response.Body.resize(fileSize);
					response.Header["Content-Length"] = std::to_string(file.read(response.Body.data(), (int64_t)response.Body.size()).gcount());
					file.close();

					auto fileType = std::filesystem::path(request.Url).extension().generic_string();
					auto result = OPENMS_MIME_TYPE.find(fileType);
					if (result == OPENMS_MIME_TYPE.end()) response.Header["Content-Type"] = "text/plain";
					else response.Header["Content-Type"] = result->second;
					return;
				}
			}
			response.Code = HTTP_STATUS_NOT_FOUND;
			response.Body = "404 Not Found";
		});
	}
}

void WebServerService::onExit()
{
	ClusterService::onExit();

	HTTPServer::shutdown();
}

void WebServerService::configureEndpoint(HTTPServer::config_t& config)
{
	config.IP = property(identity() + ".web.ip", MSString("127.0.0.1"));
	config.PortNum = property(identity() + ".web.port", 80U);
	m_MaxBodySize = property(identity() + ".web.body-size", 1024 * 1024U); // 1MB
	m_ErrorPages = property(identity() + ".web.error", MSStringMap<MSString>());
	m_StaticRoots = property(identity() + ".web.roots", MSStringMap<MSString>());
	m_StaticAlias = property(identity() + ".web.alias", MSStringMap<MSStringList>());

	config.Callback.OnOpen = [this](MSRef<IChannel> channel)
	{
		channel->getPipeline()->addFirst("inbound", MSNew<HTTPServerInboundHandler>(this));
		channel->getPipeline()->addLast("outbound", MSNew<HTTPServerOutboundHandler>(this));
		channel->getPipeline()->addBefore("outbound", "error", MSNew<WebServerErrorHandler>(this));
	};
}

void WebServerService::forward(MSString url, HTTPServer::response_t &response)
{
	// URL alias

	for (auto& pattern : m_StaticAlias)
	{
		auto& staticName = pattern.first;
		auto& staticPaths = pattern.second;
			
		// Response static files

		try
		{
			std::regex regex(staticName + "(.*)");
			std::smatch match;
			if (std::regex_search(url, match, regex))
			{
				auto filePath = match[1].str();
				for (auto& staticPath : staticPaths)
				{
					auto fullPath = std::filesystem::path(staticPath + "/" + filePath);
					if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
					{
						MS_INFO("alias %s", url.c_str());

						auto fileSize = std::filesystem::file_size(fullPath);
						if(m_MaxBodySize && m_MaxBodySize < fileSize)
						{
							response.Code = HTTP_STATUS_PAYLOAD_TOO_LARGE;
							response.Body = "413 Payload Too Large";
							return;
						}
						std::ifstream file(fullPath, std::ios::in | std::ios::binary);
						if (file.is_open())
						{
							response.Code = HTTP_STATUS_OK;
							response.Body.resize(fileSize);
							response.Header["Content-Length"] = std::to_string(file.read(response.Body.data(), (int64_t)response.Body.size()).gcount());
							file.close();

							auto fileType = std::filesystem::path(url).extension().generic_string();
							auto result = OPENMS_MIME_TYPE.find(fileType);
							if (result == OPENMS_MIME_TYPE.end()) response.Header["Content-Type"] = "text/plain";
							else response.Header["Content-Type"] = result->second;
							return;
						}
					}
				}
			}
		}
		catch (MSError& ex)
		{
			MS_ERROR("%s", ex.what());
			return;
		}
	}

	// URL roots

	for (auto& pattern : m_StaticRoots)
	{
		auto& staticName = pattern.first;
		auto& staticPath = pattern.second;

		// Response static files

		auto filePath = url;
		auto fullPath = std::filesystem::path(staticPath + filePath);
		if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
		{
			MS_INFO("root %s", url.c_str());

			auto fileSize = std::filesystem::file_size(fullPath);
			if(m_MaxBodySize && m_MaxBodySize < fileSize)
			{
				response.Code = HTTP_STATUS_PAYLOAD_TOO_LARGE;
				response.Body = "413 Payload Too Large";
				return;
			}
			std::ifstream file(fullPath, std::ios::in | std::ios::binary);
			if (file.is_open())
			{
				response.Code = HTTP_STATUS_OK;
				response.Body.resize(fileSize);
				response.Header["Content-Length"] = std::to_string(file.read(response.Body.data(), (int64_t)response.Body.size()).gcount());
				file.close();

				auto fileType = std::filesystem::path(url).extension().generic_string();
				auto result = OPENMS_MIME_TYPE.find(fileType);
				if (result == OPENMS_MIME_TYPE.end()) response.Header["Content-Type"] = "text/plain";
				else response.Header["Content-Type"] = result->second;
				return;
			}
		}
	}
	
	response.Code = HTTP_STATUS_NOT_FOUND;
	response.Body = "404 Not Found";
}

void WebServerService::redirect(MSString url, HTTPServer::response_t &response)
{
	response.Code = HTTP_STATUS_PERMANENT_REDIRECT;
	response.Header["Location"] = url;
	response.Body = "308 Permanent Redirect";
}
