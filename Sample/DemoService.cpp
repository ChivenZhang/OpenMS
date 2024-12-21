#include "DemoService.h"

class ProxyMailbox : public MailBox
{
public:
	explicit ProxyMailbox(MSRaw<IMailContext> context) : MailBox(context) { }
	void sign(IMail&& mail) override
	{
		MS_INFO("proxy: #%d %s -> %s \"%s\"", mail.SID, mail.From.c_str(), mail.To.c_str(), mail.Data.c_str());
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
	explicit LoginMailbox(MSRaw<IMailContext> context) : MailBox(context) { }
	void sign(IMail&& mail) override
	{
		auto login = TTextC<LoginInfo>::from_string(mail.Data);
		if (login.user == "admin" && login.pass == "123456")
		{
			MS_INFO("login: success");
			create<ProxyMailbox>("proxy_" + login.user);
			send({"login", "proxy_" + login.user, login.user, mail.SID });
		}
		else
		{
			MS_INFO("login: failed");
		}
	}
};

void DemoService::onInit()
{
#if 1
	auto context = AUTOWIRE(MailContext)::bean();
	context->createMailbox<LoginMailbox>("login");
	MS_INFO("test");
	for (size_t i=0; i<100; ++i)
	{
		context->sendToMailbox({"client", "login", R"({"user":"admin","pass":"123456"})" });
		// std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	MS_INFO("done");
	context->cancelMailbox("login");
	context->cancelMailbox("proxy_admin");
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