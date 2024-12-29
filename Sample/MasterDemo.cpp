#include "MasterDemo.h"
#include <OpenMS/Endpoint/RPC/RPCClient.h>
#include <OpenMS/Endpoint/RPC/RPCServer.h>

// ========================================================================================

class AuthorMailbox : public MailBox
{
public:
	explicit AuthorMailbox(IMailContextRaw context) : MailBox(context)
	{
	}

	~AuthorMailbox() override
	{
	}

	IMailTask<void> read(IMail&& mail) override
	{
		MS_INFO("read mail: %s -> %s \"%s\"", mail.From.c_str(), mail.To.c_str(), mail.Data.c_str());
		co_return;
	}
};

// ========================================================================================

class LoginMailbox : public MailBox
{
public:
	explicit LoginMailbox(IMailContextRaw context) : MailBox(context)
	{
	}

	~LoginMailbox() override
	{
	}

	IMailTask<void> read(IMail&& mail) override
	{
		MS_INFO("send mail to author...");
		send({ .To = "author", .Data = "login..." });
		co_return;
	}
};

// ========================================================================================

void MasterDemo::onInit()
{
	MasterService::onInit();

	auto context = AUTOWIRE(IMailContext)::bean();
	context->createMailbox<AuthorMailbox>("author");
	context->createMailbox<LoginMailbox>("login");
	context->sendToMailbox({ .To = "login",  .Data = R"({"user":"admin","pass":"123456"})" });
}

void MasterDemo::onExit()
{
	MasterService::onExit();
}

int main(int argc, char* argv[])
{
	return IApplication::Run<MasterDemo>(argc, argv);
}