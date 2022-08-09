#pragma once

#include "ayan/fwd.h"
#include "ayan/concepts/iterable.h"

#include <variant>
#include <string>
#include <cuchar>
#include <vector>

namespace ayan::literals
{

    /* constexpr */ std::u32string operator"" u32(const char *str, size_t len)
    {
        static std::vector<char32_t> dyn_buf{};

        if (len > dyn_buf.size())
            dyn_buf.reserve(len * 2);

        dyn_buf.clear();
        std::mbstate_t state{};
        size_t nchar = std::mbrtoc32(dyn_buf.data(), str, len, &state);
        return std::u32string(dyn_buf.data(), nchar);
    }
}

namespace ayan
{
    using MsgId = int32_t;
    using MsgStr = std::u32string;

    // 纯文本消息
    struct TextSeg
    {
        MsgStr text; // 消息内容
    };

    // qq 表情 id
    // 详见: https://github.com/kyubotics/coolq-http-api/wiki/%E8%A1%A8%E6%83%85-CQ-%E7%A0%81-ID-%E8%A1%A8
    using FaceId = int;

    // qq 表情
    struct FaceSeg
    {
        FaceId id; // 表情 id
    };

    using Qid = uint64_t; // qq 号

    // @ 某人
    struct AtSeg
    {
        Qid qq; // qq 号
    };

    // 图片消息
    struct ImageSeg
    {
        std::string file; // 图片文件名
        std::string url;  // 图片文件 url
        bool is_flash;    // 闪照 true, 否则为 false
    };

    // 语音消息
    struct RecordSeg
    {
        std::string file; // 语音文件名
        std::string url;  // 语音文件 url
        bool is_magic;    // 变声为 true, 原声为 false
    };

    // 回复消息
    struct ReplySeg
    {
        MsgId id; // 要回复的消息 id
    };

    using MsgSegment = std::variant<TextSeg,
                                    FaceSeg,
                                    AtSeg,
                                    ImageSeg,
                                    RecordSeg,
                                    ReplySeg>;

    template <typename T>
    concept IsMsgSegment = requires(T _)
    {
        {std::is_same<T, TextSeg>() ||
         std::is_same<T, FaceSeg>() ||
         std::is_same<T, AtSeg>() ||
         std::is_same<T, ImageSeg>() ||
         std::is_same<T, RecordSeg>() ||
         std::is_same<T, ReplySeg>()};
    };

    // struct SegParser
    // {
    //     static Segment parse(const json &seg);
    // };

    // struct SegDumper
    // {
    //     static json dump(const Segment &seg);

    //     json operator()(const TextSeg &text);
    //     json operator()(const FaceSeg &face);
    //     json operator()(const AtSeg &at);
    //     json operator()(const ImageSeg &image);
    //     json operator()(const RecordSeg &record);
    //     json operator()(const ReplySeg &reply);
    // };

    // template <class C>
    // concept IterableSegmentContainer = requires(C container)
    // {
    //     requires Iterable<C>;
    //     {std::is_same<decltype(*std::begin(container)), Segment>()};
    // };

    // 一条 qq 聊天消息, 内部包含多个消息段
    class Message
    {
    public:
        Message() = delete;
        Message(const Message &) = default;
        ~Message() = default;

    public:
        explicit Message(MsgSegment &&seg) : segs_()
        {
            segs_.reserve(8);
            segs_.push_back(std::forward<MsgSegment>(seg));
        }

        explicit Message(std::vector<MsgSegment> &&segs) : segs_(segs)
        {
        }

        // Message(std::string &&text);
        // Message(std::u32string &&u32text);

    public:
        // static Message from(const json &raw);

    public:
        // template <IsSegment S>
        // std::vector<S> collect() const
        // {
        //     std::vector<S> ret{};
        //     ret.reserve(_segs.size());
        //     for (auto &seg : _segs)
        //         if (std::holds_alternative<S>(seg))
        //             ret.push_back(std::get<S>(seg));
        //     return ret;
        // }

        // template <IsSegment S>
        // Message &transform(const std::function<void(S &)> &fn)
        // {
        //     for (auto &seg : _segs)
        //         if (std::holds_alternative<S>(seg))
        //             fn(seg);

        //     return *this;
        // }

        // template <IsSegment S>
        // bool check_first(const std::function<bool(S &)> &fn)
        // {
        //     for (auto &seg : _segs)
        //         if (std::holds_alternative<S>(seg))
        //             return fn(std::get<S>(seg));

        //     return false;
        // }

        // const json &as_json() const;
        // std::string dump() const;

    public:
    protected:
        std::vector<MsgSegment> segs_;
    };

    // 收到的一条消息, 包含消息 id
    class MessageView : public Message
    {
    public:
        MsgId id() const
        {
            return id_;
        }

    private:
        MsgId id_;
    };

    namespace detail
    {
        template <typename T>
        concept IsImageOrRecord = requires(T _)
        {
            {std::is_same_v<T, ImageSeg> ||
             std::is_same_v<T, RecordSeg>};
        };
    }

    class MessageBuilder : public Message
    {
    public:
        using Self = MessageBuilder;

    private:
        MessageBuilder() : Message(std::vector<MsgSegment>())
        {
            segs_.reserve(8);
        }

    public:
        MessageBuilder(const MessageBuilder &) = default;
        MessageBuilder(MessageBuilder &&) = default;

    public:
        template <IsMsgSegment Seg>
        requires detail::IsImageOrRecord<Seg>
        static Message from(Seg &&seg)
        {
            return Message(seg);
        }

        template <IsMsgSegment Seg>
        requires (Seg s)  { {detail::IsImageOrRecord<Seg>}; }
        static MessageBuilder from(Seg &&seg)
        {
            return
        }

    public:
        Self &
    };

    // class MsgBuilder
    // {
    // public:
    //     MsgBuilder() = default;
    //     MsgBuilder(const MsgBuilder &) = default;
    //     MsgBuilder(const json &raw, std::vector<Segment> &&segs);
    //     ~MsgBuilder() = default;

    // public:
    //     static MsgBuilder from();
    //     static MsgBuilder from(const std::string &u8str);
    //     static MsgBuilder from(const std::wstring &wstr);
    //     static MsgBuilder from(Segment &&seg);

    // public:
    //     MsgBuilder &text(const std::string &u8str);
    //     MsgBuilder &text(const std::wstring &wstr);
    //     MsgBuilder &face(int face_id);
    //     MsgBuilder &at(Qid qq);
    //     MsgBuilder &image_local(const std::string &local_path, bool flash = false);
    //     MsgBuilder &image_url(const std::string &url, bool flash = false, bool cache = true, bool proxy = true, unsigned timeout = 0);
    //     MsgBuilder &record_local(const std::string &local_path);
    //     MsgBuilder &record_url(const std::string &url, bool cache = true, bool proxy = true, unsigned timeout = 0);
    //     MsgBuilder &reply(MsgId msgid);
    //     MsgBuilder &push(const Segment &seg, const json &seg_dump);
    //     MsgBuilder &push(Segment &&seg, json &&seg_dump);
    //     Message build();

    // public:
    //     const json &raw();
    //     const std::vector<Segment> &segment();

    // protected:
    //     json _raw;
    //     std::vector<Segment> _segs;
    // };
}