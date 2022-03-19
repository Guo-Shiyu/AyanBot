#include "utility.h"
#include "message.h"

namespace ayan
{
	Segment SegParser::parse(const json &seg)
	{
		// todo:
		// hash map
		auto type = seg["type"].get<std::string>();
		auto &data = seg["data"];
		if (type.contains("text"))
			return TextSeg{utf8_to_wstr(data["text"].get<std::string>())};
		else if (type.contains("face"))
			return FaceSeg{std::stoi(data["id"].get<std::string>())};
		else if (type.contains("at"))
			return AtSeg{std::stoull(data["qq"].get<std::string>())};
		else if (type.contains("image"))
			return ImageSeg{
				.file = data["file"].get<std::string>(),
				.url = data["url"].get<std::string>(),
				.is_flash = false,
			};
		else if (type.contains("flash"))
			return ImageSeg{
				.file = data["file"].get<std::string>(),
				.url = data["url"].get<std::string>(),
				.is_flash = true,
			};
		else if (type.contains("record"))
			return RecordSeg{
				.file = data["file"].get<std::string>(),
				.url = data["url"].get<std::string>(),
				.is_magic = false,
				// todo:
				// unknown field
				//.is_magic = data["magic"].get<std::string>().front() == '0' ? false : true,
			};
		else if (type.contains("reply"))
			return ReplySeg{data["id"].get<MsgId>()};
		else
		{
			unsupported(type);
			return Segment{};
		}
	}

	json SegDumper::dump(const Segment &seg)
	{
		static SegDumper dumper;
		return std::visit(dumper, seg);
	}

	json SegDumper::operator()(const TextSeg &text)
	{
		json ret{
			{"type", "text"},
			{"data", {}}};
		ret["data"]["text"] = wstr_to_utf8(text.text);
		return ret;
	}

	json SegDumper::operator()(const FaceSeg &face)
	{
		json ret{
			{"type", "face"},
			{"data", {}}};
		ret["data"]["id"] = face.id;
		return ret;
	}

	json SegDumper::operator()(const AtSeg &at)
	{
		json ret{
			{"type", "at"},
			{"data", {}}};
		ret["data"]["qq"] = at.qq;
		return ret;
	}

	json SegDumper::operator()(const ImageSeg &image)
	{
		json ret{
			{"type", "image"},
			{"data", {}}};
		ret["data"]["file"] = image.file;
		ret["data"]["url"] = image.url;
		if (image.is_flash == true)
			ret["data"]["type"] = "flash";
		return ret;
	}

	json SegDumper::operator()(const RecordSeg &rec)
	{
		json ret{
			{"type", "record"},
			{"data", {}}};
		ret["data"]["file"] = rec.file;
		ret["data"]["url"] = rec.url;
		// todo:
		// unknown field
		// ret["data"]["flash"] = rec.is_magic ? 1 : 0;
		return ret;
	}

	json SegDumper::operator()(const ReplySeg &reply)
	{
		json ret{
			{"type", "reply"},
			{"data", {}}};
		ret["data"]["id"] = reply.id;
		return ret;
	}

	MsgBuilder::MsgBuilder(const json &raw, std::vector<Segment> &&segs)
		: _raw(raw), _segs(segs)
	{
	}

	MsgBuilder MsgBuilder::from()
	{
		return MsgBuilder();
	}

	MsgBuilder MsgBuilder::from(const std::string &u8str)
	{
		TextSeg text{utf8_to_wstr(u8str)};
		json msg = json::array();
		msg.push_back(SegDumper::dump(text));
		return MsgBuilder(std::move(msg), {text});
	}

	MsgBuilder MsgBuilder::from(const std::wstring &wstr)
	{
		TextSeg text{wstr};
		json msg = json::array();
		msg.push_back(SegDumper::dump(text));
		return MsgBuilder(std::move(msg), {text});
	}

	MsgBuilder MsgBuilder::from(Segment &&seg)
	{
		MsgBuilder ret{};
		json dump = SegDumper::dump(seg);
		ret.push(std::forward<Segment>(seg), std::move(dump));
		return ret;
	}

	MsgBuilder &MsgBuilder::text(const std::string &u8str)
	{
		return text(utf8_to_wstr(u8str));
	}

	MsgBuilder &MsgBuilder::text(const std::wstring &wstr)
	{
		TextSeg text{wstr};
		return push(text, SegDumper::dump(text));
	}

	MsgBuilder &MsgBuilder::face(int face_id)
	{
		FaceSeg face{face_id};
		return push(face, SegDumper::dump(face));
	}

	MsgBuilder &MsgBuilder::at(Qid qq)
	{
		AtSeg at{qq};
		return push(at, SegDumper::dump(at));
	}

	MsgBuilder &MsgBuilder::image_local(const std::string &local_path, bool flash)
	{
		ImageSeg image{
			.file = std::string("file:///").append(local_path),
			.url = "", /// empty
			.is_flash = flash,
		};
		return push(image, SegDumper::dump(image));
	}

	MsgBuilder &MsgBuilder::image_url(const std::string &url, bool flash, bool cache, bool proxy, unsigned timeout)
	{
		ImageSeg image{
			.file = url,
			.url = "", /// empty
			.is_flash = flash,
		};

		auto packet = SegDumper::dump(image);
		auto &data = packet["data"];
		if (cache)
			data["cache"] = true;
		if (proxy)
			data["proxy"] = true;
		if (timeout != 0)
			data["timeout"] = timeout;

		return push(image, std::move(packet));
	}

	MsgBuilder &MsgBuilder::record_local(const std::string &local_path)
	{
		RecordSeg record{
			.file = local_path,
			.url = "", /// empty
			.is_magic = false,
		};
		return push(record, SegDumper::dump(record));
	}

	MsgBuilder &MsgBuilder::record_url(const std::string &url, bool cache, bool proxy, unsigned timeout)
	{
		RecordSeg record{
			.file = url,
			.url = "", /// empty
			.is_magic = false,
		};

		auto packet = SegDumper::dump(record);
		auto &data = packet["data"];
		if (cache)
			data["cache"] = true;
		if (proxy)
			data["proxy"] = true;
		if (timeout != 0)
			data["timeout"] = timeout;

		return push(record, std::move(packet));
	}

	MsgBuilder &MsgBuilder::reply(MsgId msgid)
	{
		ReplySeg reply{msgid};
		return push(reply, SegDumper::dump(reply));
	}

	MsgBuilder &MsgBuilder::push(const Segment &seg, const json &seg_dump)
	{
		_segs.push_back(seg);
		_raw.push_back(seg_dump);
		return *this;
	}

	MsgBuilder &MsgBuilder::push(Segment &&seg, json &&seg_dump)
	{
		_segs.push_back(seg);
		_raw.push_back(seg_dump);
		return *this;
	}

	Message MsgBuilder::build()
	{
		return Message(_raw, std::move(_segs));
	}

	const json &MsgBuilder::raw()
	{
		return _raw;
	}
	const std::vector<Segment> &MsgBuilder::segment()
	{
		return _segs;
	}

	Message::Message(const std::u8string &text)
	{
		// fool impl
		*this = MsgBuilder::from(std::to_string(text)).build();
	}

	Message::Message(const std::string &text)
	{
		// fool impl
		*this = MsgBuilder::from(text).build();
	}

	Message::Message(const std::wstring &text)
	{
		// fool impl
		*this = MsgBuilder::from(text).build();
	}

	Message::Message(const json &raw, std::vector<Segment> &&segs)
		: MsgBuilder(raw, std::forward<std::vector<Segment>>(segs))
	{
	}

	Message Message::from(const json &raw)
	{
		std::vector<Segment> ret;
		for (auto &elem : raw)
			ret.push_back(SegParser::parse(elem));
		return Message(raw, std::move(ret));
	}

	const json &Message::as_json() const
	{
		return _raw;
	}

	std::string Message::dump() const
	{
		return _raw.dump();
	}

}