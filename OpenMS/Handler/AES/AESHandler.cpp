#include "AESHandler.h"
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>

AESInboundHandler::AESInboundHandler(config_t const& config)
	:
	m_Config(config)
{
}

bool AESInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	auto& key = m_Config.Key;
	auto& iv = m_Config.IV;

	auto input = (uint8_t*)event->Message.data();
	auto length = event->Message.size();
	TString result;

	int ret = MBEDTLS_ERR_AES_BAD_INPUT_DATA;
	mbedtls_aes_context aes_ctx;
	mbedtls_aes_init(&aes_ctx);
	do
	{
		if (ret = mbedtls_aes_setkey_dec(&aes_ctx, key.data(), (int)key.size() * 8))break;
		size_t count0 = length / 16 * 16;
		size_t count1 = count0 + ((length % 16) ? 16 : 0);
		result.resize(count1);
		if (ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, count0, iv.data(), input, (uint8_t*)result.data())) break;
		if (count0 != count1)
		{
			uint8_t block[16]{ 0 };
			::memcpy(block, input + count0, length - count0);
			ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, sizeof(block), iv.data(), block, (uint8_t*)result.data() + count0);
		}
	} while (0);
	mbedtls_aes_free(&aes_ctx);

	if (ret == 0)
	{
		event->Message = result.c_str();
		return true;
	}
	return false;
}

AESOutboundHandler::AESOutboundHandler(config_t const& config)
	:
	m_Config(config)
{
}

bool AESOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	auto& key = m_Config.Key;
	auto& iv = m_Config.IV;

	auto input = (uint8_t*)event->Message.data();
	auto length = event->Message.size();
	TString result;

	int ret = MBEDTLS_ERR_AES_BAD_INPUT_DATA;
	mbedtls_aes_context aes_ctx;
	mbedtls_aes_init(&aes_ctx);
	do
	{
		if (ret = mbedtls_aes_setkey_enc(&aes_ctx, key.data(), (int)key.size() * 8))break;
		size_t count0 = length / 16 * 16;
		size_t count1 = count0 + ((length % 16) ? 16 : 0);
		result.resize(count1);
		if (ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, count0, iv.data(), input, (uint8_t*)result.data())) break;
		if (count0 != count1)
		{
			uint8_t block[16]{ 0 };
			::memcpy(block, input + count0, length - count0);
			ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, sizeof(block), iv.data(), block, (uint8_t*)result.data() + count0);
		}
	} while (0);
	mbedtls_aes_free(&aes_ctx);

	if (ret == 0)
	{
		event->Message = result;
		return true;
	}
	return false;
}