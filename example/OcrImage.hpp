#pragma once

#include "../Ayan/core/service.h"
#include "../Ayan/core/apicall.h"

#include <chrono> 

/*
 *  unstable example
 *
 *  do not use this class
 */

class OcrImageTest : public MessageService<OcrImageTest>
{
public:
    

public:
	std::string_view name() const override
	{
		return "ocr-image";
	}

	bool trig(Bot& bot, PrivateMessage& msg)
	{
		return not msg.message.collect<ImageSeg>().empty();
	}

	bool trig(Bot& bot, GroupMessage& msg)
	{
		return false;
	}

	void act(Bot& bot, PrivateMessage& msg)
	{
        using namespace  std::literals::chrono_literals;

		auto image = msg.message.collect<ImageSeg>().front();
		auto fu_ocr = bot.api().ocr_image(image.file);

		fu_ocr.wait_for(8s);

		auto ocr = fu_ocr.get();

		auto reply = MsgBuilder::from(L"")
			.text(ocr.language)
			.text(L"")
			.text(ocr.texts.front().text)
			.text(L"")
			.text(std::to_string(ocr.texts.front().confidence))
			.build();

		bot.api().send_private_msg({ msg.sender.user_id, reply });
	}

	using MessageService<OcrImageTest>::act;
};