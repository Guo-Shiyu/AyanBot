#pragma once

#include "ayan/import/json.h"
#include "ayan/import/optional.h"

#include "ayan/concepts/sumtype.h"

#include <limits>

namespace onebot {
using MsgId      = int32_t;
using MsgStr     = std::u32string;
using MsgStrView = std::u32string_view;

// 纯文本消息
struct TextSeg {
  MsgStr text; // 消息内容
};

// qq 表情 id
// 详见:
// https://github.com/kyubotics/coolq-http-api/wiki/%E8%A1%A8%E6%83%85-CQ-%E7%A0%81-ID-%E8%A1%A8
using FaceId = int;

// qq 表情
struct FaceSeg {
  FaceId id; // 表情 id
};

using Qid = uint64_t; // qq 号

// @ 某人
struct AtSeg {
  Qid qq; // 被 at 的 qq 号
};

// 图片消息
struct ImageSeg {
  std::string file;     // 图片文件名
  std::string url;      // 图片文件 url
  bool        is_flash; // 闪照 true, 否则为 false
};

// 语音消息
struct RecordSeg {
  std::string file;     // 语音文件名
  std::string url;      // 语音文件 url
  bool        is_magic; // 变声 true, 原声为 false
};

// 回复消息
struct ReplySeg {
  MsgId id; // 要回复的消息 id
};

using MsgSegment = std::variant<TextSeg, FaceSeg, AtSeg, ImageSeg, RecordSeg, ReplySeg>;

template <typename T>
concept IsMsgSegment = requires(T _) {
                         {
                           std::is_same<T, TextSeg>() || std::is_same<T, FaceSeg>() ||
                               std::is_same<T, AtSeg>() || std::is_same<T, ImageSeg>() ||
                               std::is_same<T, RecordSeg>() || std::is_same<T, ReplySeg>()
                         };
                       };

struct MsgSegDumper {
  static json dump(const MsgSegment &seg);

  json operator()(const TextSeg &text);
  json operator()(const FaceSeg &face);
  json operator()(const AtSeg &at);
  json operator()(const ImageSeg &image);
  json operator()(const RecordSeg &record);
  json operator()(const ReplySeg &reply);
};

// 主动构造的 qq 聊天消息, 内部包含多个消息段
class Message {
protected:
  constexpr static auto kDefaultReserveSize = 8U;

public:
  Message()                = delete;
  Message(const Message &) = default;
  ~Message()               = default;

public:
  explicit Message(MsgSegment &&seg) : segs_() {
    segs_.reserve(kDefaultReserveSize);
    segs_.push_back(std::forward<MsgSegment>(seg));
  }

  explicit Message(std::vector<MsgSegment> &&segs) : segs_(segs) {}

public:
  size_t len() {
    return segs_.size();
  }

  // dump message to onenot array message format
  json dump() const {
    json dmp = json::array();
    for (auto &seg : this->segs_) {
      dmp.push_back(MsgSegDumper::dump(seg));
    }
    return dmp;
  }

protected:
  std::vector<MsgSegment> segs_;
};

// 收到的一条消息, 包含消息 id
class MessageView : public Message {
public:
  constexpr static MsgId kNoSignificantId = std::numeric_limits<MsgId>::max();

  using Self = MessageView;

public:
  MessageView()                    = delete;
  MessageView(const MessageView &) = default;
  ~MessageView()                   = default;

public:
  MessageView(Message &&msg, MsgId id) : Message(std::forward<Message>(msg)), id_(id) {}

  static Self from(Message &&msg, MsgId id = kNoSignificantId) {
    return MessageView(std::forward<Message>(msg), id);
  }

public:
  MsgId id() const {
    return id_;
  }

  /// 保留前 n 个消息段， 在消息段数量小于 n 时， 不进行操作
  Self &take(size_t n) {
    if (n < segs_.size())
      segs_.resize(n);
    return *this;
  }

  /// 只保留类型为 Seg 的消息段
  template <IsMsgSegment Seg>
  Self &take() {
    std::erase_if(segs_, [](const MsgSegment &seg) {
      return not std::holds_alternative<Seg>(seg);
    });
    return *this;
  }

  /// 保留类型为 Seg 并且 cond 返回 true 的消息段
  template <IsMsgSegment Seg>
  Self &take_if(const std::function<bool(const Seg &)> &cond) {
    std::erase_if(segs_, [&](const MsgSegment &seg) {
      return not std::holds_alternative<Seg>(seg) || cond(std::get<Seg>(seg));
    });
    return *this;
  }

  /// 过滤掉所有类型为 Seg 且 cond 返回 true 的的消息段
  template <IsMsgSegment Seg>
  Self &filter(const std::function<bool(const Seg &)> &cond) {
    std::erase_if(segs_, [&](const MsgSegment &seg) {
      return std::holds_alternative<Seg>(seg) && cond(std::get<Seg>(seg));
    });
    return *this;
  }

  /// 将所有相邻的 text 段合并
  Self &flatten() {
    for (auto i = 0U; i < segs_.size(); i++) {
      auto &cur = segs_.at(i);
      if (std::holds_alternative<TextSeg>(cur)) {
        auto &src = std::get<TextSeg>(cur).text;
        for (auto j = i + 1; j < segs_.size(); j++) {
          auto &nxt = segs_.at(j);
          if (std::holds_alternative<TextSeg>(nxt)) {
            src.append(std::get<TextSeg>(nxt).text);

            // swap to end and pop back
            std::swap(segs_[j], segs_.back());
            src.pop_back();
          } else
            break;
        }
      }
    }

    return *this;
  }

  template <IsMsgSegment Seg, template <typename S> typename Container = std::vector>
  Container<Seg> collect() const {
    Container<Seg> ret{};
    /// TODO:
    // if constexpr (std::is_same_v<Container, std::vector>())
    //     ret.reserve(segs_.size());

    for (auto &seg : segs_)
      if (std::holds_alternative<Seg>(seg))
        ret.emplace(std::get<Seg>(seg));
    return ret;
  }

  template <IsMsgSegment Seg>
  Self &transform(const std::function<void(const Seg &)> &trans) {
    for (auto &seg : segs_)
      if (std::holds_alternative<Seg>(seg))
        trans(seg);
    return *this;
  }

  template <IsMsgSegment FromSeg, IsMsgSegment ToSeg>
  Self &transform_to(const std::function<ToSeg(const FromSeg &&)> &trans) {
    for (auto &seg : segs_)
      if (std::holds_alternative<FromSeg>(seg))
        seg.emplace(trans(std::move(seg)));

    return *this;
  }

  template <IsMsgSegment Seg>
  Optional<bool> check_first(const std::function<bool(const Seg &)> &cond) {
    for (auto &seg : segs_)
      if (std::holds_alternative<Seg>(seg))
        return make_optional(cond(std::get<Seg>(seg)));
    return NullOpt;
  }

  template <IsMsgSegment Seg>
  Optional<Seg> take_first() {
    if (std::holds_alternative<Seg>(segs_.front())) {
      auto seg = std::get<Seg>(segs_.front());
      return make_optional(std::move(seg));
    }
    return NullOpt;
  }

private:
  MsgId id_;
};

struct MessageParser {
  static Message from_raw(const json &seg_array);
};

// pimpl idom
class MsgBuilderImpl;

// 消息构造器
class MessageBuilder {
public:
  using Self = MessageBuilder;
  using Impl = MsgBuilderImpl;

private:
  MessageBuilder()                       = delete;
  MessageBuilder(const MessageBuilder &) = delete;
  MessageBuilder(std::vector<MsgSegment> &&segs);

public:
  MessageBuilder(MessageBuilder &&rhs) = default;

  /// 从任一单独的消息段中构造消息
  template <IsMsgSegment Seg>
  static MessageBuilder from(Seg &&s) {
    std::vector<MsgSegment> segs{};
    segs.push_back(std::move(s));
    return MessageBuilder(std::move(segs));
  }

  /// 语音消息只能单独成段
  template <IsMsgSegment Seg>
    requires std::is_same_v<Seg, RecordSeg>
  static Message from(Seg &&seg) {
    return Message(seg);
  }

  /// 语音消息
  static Message record_local(const std::string &local_path);
  static Message record_url(
      const std::string &url, bool cache = true, bool proxy = true, unsigned timeout = 0);

public:
  // 添加文本段
  Self &text(std::string_view str);
  Self &text(MsgStrView str);

  // 添加表情
  Self &face(FaceId face_id);

  // 添加 @
  Self &at(Qid qq);

  // 添加回复
  Self &reply(MsgId msgid);

  // 从本地路径加载图片
  Self &image_local(const std::string &local_path, bool flash = false);

  // 同 url 中加载图片
  Self &image_url(
      const std::string &url, bool flash = false, bool cache = true, bool proxy = true,
      unsigned timeout = 0);

  Message build();

private:
  std::unique_ptr<MsgBuilderImpl> impl_;
};
} // namespace onebot
