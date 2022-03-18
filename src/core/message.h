#pragma once

#include "hv/json.hpp"
#include "fwd.h"
 
#include <variant>
#include <string>

namespace ayan
{
    using json = nlohmann::json;

    struct TextSeg
    {
        std::wstring text;
    };

    struct FaceSeg
    {
        int id;
    };

    struct AtSeg
    {
        Qid qq;
    };

    struct ImageSeg
    {
        std::string file, url;
        bool is_flash;
    };

    struct RecordSeg
    {
        std::string file, url;
        bool is_magic;
    };

    struct ReplySeg
    {
        MsgId id;
    };

    using Segment = std::variant<TextSeg,
                                 FaceSeg,
                                 AtSeg,
                                 ImageSeg,
                                 RecordSeg,
                                 ReplySeg>;

    template <typename T>
    concept IsSegment = requires(T _)
    {
        {std::is_same<T, TextSeg>() ||
         std::is_same<T, FaceSeg>() ||
         std::is_same<T, AtSeg>() ||
         std::is_same<T, ImageSeg>() ||
         std::is_same<T, RecordSeg>() ||
         std::is_same<T, ReplySeg>()};
    };

    struct SegParser
    {
        static Segment parse(const json &seg);
    };

    struct SegDumper
    {
        static json dump(const Segment &seg);

        json operator()(const TextSeg &text);
        json operator()(const FaceSeg &face);
        json operator()(const AtSeg &at);
        json operator()(const ImageSeg &image);
        json operator()(const RecordSeg &record);
        json operator()(const ReplySeg &reply);
    };

    template <class C>
    concept IterableSegmentContainer = requires(C container)
    {
        requires Iterable<C>;
        {std::is_same<decltype(*std::begin(container)), Segment>()};
    };

    class Message;
    class MsgBuilder
    {
    public:
        MsgBuilder() = default;
        MsgBuilder(const MsgBuilder &) = default;
        MsgBuilder(const json &raw, std::vector<Segment> &&segs);
        ~MsgBuilder() = default;

    public:
        static MsgBuilder from();
        static MsgBuilder from(const std::string &u8str);
        static MsgBuilder from(const std::wstring &wstr);
        static MsgBuilder from(Segment &&seg);

    public:
        MsgBuilder &text(const std::string &u8str);
        MsgBuilder &text(const std::wstring &wstr);
        MsgBuilder &face(int face_id);
        MsgBuilder &at(Qid qq);
        MsgBuilder &image_local(const std::string &local_path, bool flash = false);
        MsgBuilder &image_url(const std::string &url, bool flash = false, bool cache = true, bool proxy = true, unsigned timeout = 0);
        MsgBuilder &record_local(const std::string &local_path);
        MsgBuilder &record_url(const std::string &url, bool cache = true, bool proxy = true, unsigned timeout = 0);
        MsgBuilder &reply(MsgId msgid);
        MsgBuilder &push(const Segment &seg, const json &seg_dump);
        MsgBuilder &push(Segment &&seg, json &&seg_dump);
        Message build();

    public:
        const json &raw();
        const std::vector<Segment> &segment();

    protected:
        json _raw;
        std::vector<Segment> _segs;
    };

    class Message : private MsgBuilder
    {
    public:
        friend class MsgBuilder;

    public:
        Message() = delete;
        Message(const Message &) = default;
        ~Message() = default;

    public:
        Message(const std::u8string &text);
        Message(const std::string &text);
        Message(const std::wstring &text);

    protected:
        explicit Message(const json &raw, std::vector<Segment> &&segs);

    public:
        static Message from(const json &raw);

    public:
        template <IsSegment S>
        std::vector<S> collect() const
        {
            std::vector<S> ret{};
            ret.reserve(_segs.size());
            for (auto &seg : _segs)
                if (std::holds_alternative<S>(seg))
                    ret.push_back(std::get<S>(seg));
            return ret;
        }

        template<IsSegment S>
        Message& transform(const std::function<void(S&)>& fn)
        {
            for (auto &seg : _segs)
                if (std::holds_alternative<S>(seg))
                    fn(seg);
                    
            return *this;            
        }

        template<IsSegment S>
        bool check_first(const std::function<bool(S&)>& fn)
        {
            for (auto &seg : _segs)
                if (std::holds_alternative<S>(seg))
                    return fn(std::get<S>(seg));
            
            return false;
        }

        const json &as_json() const;
        std::string dump() const;

    public:
        using MsgBuilder::segment;
    };
}