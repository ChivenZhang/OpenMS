#include "DemoService.h"
#include <coroutine>

class ProxyMailbox : public MailBox
{
public:
	explicit ProxyMailbox(IMailContextRaw context) : MailBox(context) { }

	IMailResult sign(IMail&& mail) override
	{
		while (true)
		{
			MS_INFO("proxy: #%d %s -> %s \"%s\"", mail.SID, mail.From.c_str(), mail.To.c_str(), mail.Data.c_str());
			co_yield {};
		}
	}
};

struct LoginInfo
{
	MSString user;
	MSString pass;
	OPENMS_TYPE(LoginInfo, user, pass);
};

class LoginMailbox : public MailBox
{
public:
	explicit LoginMailbox(IMailContextRaw context) : MailBox(context) { }

	IMailResult sign(IMail&& mail) override
	{
		auto login = TTextC<LoginInfo>::from_string(mail.Data);
		if (login.pass == "123456")
		{
			MS_INFO("login: success");
			create<ProxyMailbox>("proxy_" + login.user);
			send({"login", "proxy_" + login.user, login.user, mail.SID });
		}
		else
		{
			MS_INFO("login: failed");
		}
		co_return;
	}
};

void DemoService::onInit()
{
#if 1
	auto context = AUTOWIRE(MailContext)::bean();
	context->createMailbox<LoginMailbox>("login");
	MS_INFO("test");
	context->sendToMailbox({"client", "login", R"({"user":"admin1","pass":"123456"})" });
	context->sendToMailbox({"client", "login", R"({"user":"admin2","pass":"123456"})" });
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