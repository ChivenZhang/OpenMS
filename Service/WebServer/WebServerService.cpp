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

MSString WebServerService::identity() const
{
	return "webserver";
}

void WebServerService::onInit()
{
	ClusterService::onInit();

	HTTPServer::startup();

	for (auto& pattern : m_StaticRoots)
	{
		auto& staticName = pattern.first;
		auto& staticPaths = pattern.second;
		bind_get(staticName + ".*", [=, this](request_t const& request, response_t& response)
		{
			MS_INFO("request %s", request.Url.c_str());

			// Response static files

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
							std::ifstream file(fullPath, std::ios::in | std::ios::binary);
							if (file.is_open())
							{
								response.Code = HTTP_STATUS_OK;
								response.Body.resize(std::min<size_t>(m_MaxBodySize, fileSize));
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
	m_MaxBufferSize = property(identity() + ".web.buffer-size", 1024 * 1024U); // 1MB
	m_StaticRoots = property(identity() + ".web.roots", MSStringMap<MSStringList>());
	m_StaticAlias = property(identity() + ".web.alias", MSStringMap<MSString>());
	// TODO: url alias
}
