#include "ayan/import/json.h"

#include "ayan/onebot/message.h"
#include "ayan/utils/container.h"
#include "ayan/utils/util.h"

namespace onebot {

json MsgSegDumper::dump(const MsgSegment &seg) {
  MsgSegDumper dumper;
  return std::visit(dumper, seg);
}

json MsgSegDumper::operator()(const TextSeg &text) {
  json ret{
      {"type", "text"},
      {"data", {}    }
  };
  ret["data"]["text"] = utf8::utf32to8(text.text);
  return ret;
}

json MsgSegDumper::operator()(const FaceSeg &face) {
  json ret{
      {"type", "face"},
      {"data", {}    }
  };
  ret["data"]["id"] = face.id;
  return ret;
}

json MsgSegDumper::operator()(const AtSeg &at) {
  json ret{
      {"type", "at"},
      {"data", {}  }
  };
  ret["data"]["qq"] = at.qq;
  return ret;
}

json MsgSegDumper::operator()(const ImageSeg &image) {
  json ret{
      {"type", "image"},
      {"data", {}     }
  };
  ret["data"]["file"] = image.file;
  ret["data"]["url"]  = image.url;
  if (image.is_flash == true)
    ret["data"]["type"] = "flash";
  return ret;
}

json MsgSegDumper::operator()(const RecordSeg &rec) {
  json ret{
      {"type", "record"},
      {"data", {}      }
  };
  ret["data"]["file"] = rec.file;
  ret["data"]["url"]  = rec.url;
  // todo:
  // unknown field
  // ret["data"]["flash"] = rec.is_magic ? 1 : 0;
  return ret;
}

json MsgSegDumper::operator()(const ReplySeg &reply) {
  json ret{
      {"type", "reply"},
      {"data", {}     }
  };
  ret["data"]["id"] = reply.id;
  return ret;
}

using ParseSegFn = std::function<MsgSegment(const json &)>;

// flash 和 image 两种情况都会返回 image 对象
constexpr size_t DealOnebotMsgSegCases = std::variant_size_v<MsgSegment> + 1;

const static ayan::KDenseMap<std::string_view, ParseSegFn, DealOnebotMsgSegCases>
    ParseSegMap = {
        {"text",
         [](const json &data) {
           return TextSeg{
               .text = utf8::utf8to32(data["text"].get<std::string_view>()),
           };
         }                                                                       },

        {"face",
         [](const json &data) {
           return FaceSeg{.id = std::stoi(data["id"].get<std::string>())};
         }                                                                       },
        {"at",
         [](const json &data) {
           return AtSeg{.qq = std::stoull(data["qq"].get<std::string>())};
         }                                                                       },
        {"image",
         [](const json &data) {
           return ImageSeg{
               .file     = data["file"].get<std::string>(),
               .url      = data["url"].get<std::string>(),
               .is_flash = false,
           };
         }                                                                       },
        {"flash",
         [](const json &data) {
           return ImageSeg{
               .file     = data["file"].get<std::string>(),
               .url      = data["url"].get<std::string>(),
               .is_flash = true,
           };
         }                                                                       },
        {"record",
         [](const json &data) {
           return RecordSeg{
               .file     = data["file"].get<std::string>(),
               .url      = data["url"].get<std::string>(),
               .is_magic = false,
               /// TODO:
               /// unknown field about flash flag
               /// .is_magic = data["magic"].get<std::string>().front() == '0' ?
               /// false : true,
           };
         }                                                                       },
        {"reply",
         [](const json &data) { return ReplySeg{.id = data["id"].get<MsgId>()}; }},
};

Message MessageParser::from_raw(const json &seg_array) {
  std::vector<MsgSegment> segs{};

  size_t count = seg_array.size();
  segs.reserve(count);

  // 解析每一个消息段
  for (auto i = 0U; i < count; i++) {
    auto  type = seg_array[i]["type"].get<std::string_view>();
    auto &data = seg_array[i]["data"];
    segs.push_back(std::invoke(ParseSegMap.at(type), data));
  }

  return Message(std::move(segs));
}
} // namespace onebot