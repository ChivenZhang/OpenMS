#include "DemoService.h"
#include <OpenMS/Endpoint/RPC/RPCClient.h>
#include <OpenMS/Endpoint/RPC/RPCServer.h>

// ========================================================================================

class AuthorMailbox : public MailBox, public RPCServer
{
public:
	explicit AuthorMailbox(IMailContextRaw context) : MailBox(context)
	{
		RPCServer::startup();
		bind("author", []()->MSString
		{
			return "success";
		});
	}

	~AuthorMailbox() override
	{
		RPCServer::shutdown();
	}

	IMailResult read(IMail&& mail) override
	{
		MS_INFO("read mail: %s -> %s \"%s\"", mail.From.c_str(), mail.To.c_str(), mail.Data.c_str());
		co_return;
	}

	void configureEndpoint(config_t& config) override
	{
		config.IP = "127.0.0.1";
		config.PortNum = 8080;
	}
};

// ========================================================================================

class LoginMailbox : public MailBox, public RPCClient
{
public:
	explicit LoginMailbox(IMailContextRaw context) : MailBox(context)
	{
		RPCClient::startup();
	}

	~LoginMailbox() override
	{
		RPCClient::shutdown();
	}

	IMailResult read(IMail&& mail) override
	{
		MS_INFO("send mail to author...");
		send({ .To = "author", .Data = "login..." });

		auto result = call<MSString>("author", 1000);
		MS_INFO("authority: %s", result.c_str());

		co_return;
	}

	void configureEndpoint(config_t& config) override
	{
		config.IP = "127.0.0.1";
		config.PortNum = 8080;
	}
};

// ========================================================================================

void DemoService::onInit()
{
#if 1 // Test
	auto context = AUTOWIRE(MailContext)::bean();
	context->createMailbox<AuthorMailbox>("author");
	context->createMailbox<LoginMailbox>("login");
	context->sendToMailbox({ .To = "login",  .Data = R"({"user":"admin","pass":"123456"})" });
#endif

	// auto server = AUTOWIRE(DemoServer)::bean();
	// auto client = AUTOWIRE(DemoClient)::bean();
	// server->startup();
	// client->startup();
}

void DemoService::onExit()
{
	// auto server = AUTOWIRE(DemoServer)::bean();
	// auto client = AUTOWIRE(DemoClient)::bean();
	// server->shutdown();
	// client->shutdown();
}

int main(int argc, char* argv[])
{
	return IApplication::Run<DemoService>(argc, argv);
}