#include "DemoService.h"
#include <OpenMS/Endpoint/RPC/RPCClient.h>

class ProxyMailbox : public MailBox
{
public:
	explicit ProxyMailbox(IMailContextRaw context) : MailBox(context) { }

	IMailResult sign(IMailResult coro, IMail&& mail) override
	{
		MS_INFO("proxy: #%d %s -> %s \"%s\"", mail.SID, mail.From.c_str(), mail.To.c_str(), mail.Data.c_str());
		co_return;
	}
};

// ========================================================================================

struct LoginInfo
{
	MSString user;
	MSString pass;
	OPENMS_TYPE(LoginInfo, user, pass);
};

class LoginMailbox : public MailBox, public AUTOWIRE(RPCClient)
{
public:
	explicit LoginMailbox(IMailContextRaw context) : MailBox(context) {}

	IMailResult sign(IMailResult coro, IMail&& mail) override
	{
		auto login = [](LoginInfo info)->IMailTask<MSString>
		{
			for (auto i=0; i<10; ++i) co_yield "busy";
			co_return "success";
		};

		auto _login = login(TTextC<LoginInfo>::from_string(mail.Data));
		while (true)
		{
			auto result = co_await _login;
			if (result == "success")
			{
				send({"login", "proxy", "login success" });
				break;
			}
			if (result == "busy") continue;
			break;
		}
	}
};

// ========================================================================================

void DemoService::onInit()
{
#if 1
	auto context = AUTOWIRE(MailContext)::bean();
	MS_INFO("test");
	context->createMailbox<LoginMailbox>("login");
	context->createMailbox<ProxyMailbox>("proxy");
	context->sendToMailbox({"client", "login", R"({"user":"admin","pass":"123456"})" });
	MS_INFO("done");
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