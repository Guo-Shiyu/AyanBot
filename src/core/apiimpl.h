
#ifdef UNCOMMON_INCLUDE

#ifndef AYAN_API
#define AYAN_API(_)
#endif

#ifndef API_DECLARE
#define API_DECLARE(...)
#endif

#else

#include "bot.h"

#include <future>

#define AYAN_API(name) struct name
#define API_DECLARE(...) {__VA_ARGS__};

namespace ayan
{
	template <typename T>
	using ApiRetHook = std::function<void(T &&)>;

	struct FriendDescriptor
	{
		Qid user_id;
		std::string nickname;
		std::string remark;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FriendDescriptor, user_id, nickname, remark)

	struct GroupDescriptor
	{
		Qid group_id;
		std::string group_name, group_memo;
		uint32_t group_create_time, group_level;
		int32_t member_count, max_member_count;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GroupDescriptor, group_id, group_name, group_memo, group_create_time, group_level, member_count, max_member_count)

	struct GroupMemberDescriptor
	{
		Qid group_id, user_id;
		std::string nickname, card, sex, area, level, title, role;
		int32_t age, join_time, last_sent_time;
		int64_t title_expire_time, shut_up_timestamp;
		bool unfriendly, card_changeable;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GroupMemberDescriptor, group_id, user_id, nickname, card, sex, area, level, title, role, age, join_time, last_sent_time, title_expire_time, shut_up_timestamp, unfriendly, card_changeable)

	struct Position
	{
		int x, y;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, x, y)

	struct TextDetection
	{
		int confidence;
		std::array<Position, 4> coordinates;
		std::string text;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextDetection, confidence, coordinates, text)

	struct OcrResult
	{
		std::string language;
		std::vector<TextDetection> texts;
	};
}

namespace Hook
{
	inline constexpr auto _ = [](auto &&_) {};
}

using namespace ayan;

#pragma warning(disable : 5030)
#pragma warning(disable : 5222)
#endif // UNCOMMON_INCLUDE

AYAN_API(_SendPrivateMessage)
API_DECLARE(
	auto send_private_msg(Qid id, const Message &msg)->std::future<MsgId>;
	ApiCall & send_private_msg(std::tuple<Qid, const Message &> && params, const ApiRetHook<MsgId> &hook = Hook::_);)

AYAN_API(_SendGroupMessage)
API_DECLARE(
	auto send_group_msg(Qid id, const Message &msg)->std::future<MsgId>;
	ApiCall & send_group_msg(std::tuple<Qid, const Message &> && params, const ApiRetHook<MsgId> &hook = Hook::_);)

AYAN_API(_DeleteMessage)
API_DECLARE(
	auto delete_msg(MsgId id)->std::future<void>;
	ApiCall & delete_msg(std::tuple<MsgId> && params);)

AYAN_API(_GetFriendList)
API_DECLARE(
	using CallResult = std::vector<FriendDescriptor>;

	auto get_friend_list()->std::future<CallResult>;
	ApiCall & get_friend_list(const ApiRetHook<CallResult> &hook);)

AYAN_API(_DeleteFriend)
API_DECLARE(
	auto delete_friend(Qid fid)->std::future<void>;
	ApiCall & delete_friend(std::tuple<Qid> && params);)

AYAN_API(_GetGroupInfo)
API_DECLARE(
	auto get_group_info(Qid group_id)->std::future<GroupDescriptor>;
	ApiCall & get_group_info(std::tuple<Qid> && params, const ApiRetHook<GroupDescriptor> &hook = Hook::_);)

AYAN_API(_GetGroupList)
API_DECLARE(
	using CallResult = std::vector<GroupDescriptor>;

	auto get_group_list()->std::future<CallResult>;
	ApiCall & get_group_list(const ApiRetHook<CallResult> &hook);)

AYAN_API(_GetGroupMemberInfo)
API_DECLARE(
	auto get_group_member_info(Qid group, Qid user)->std::future<GroupMemberDescriptor>;
	ApiCall & get_group_member_info(std::tuple<Qid, Qid> && params, const ApiRetHook<GroupMemberDescriptor> &hook = Hook::_);)

AYAN_API(_GetGroupMemberList)
API_DECLARE(
	using CallResult = std::vector<GroupMemberDescriptor>;

	auto get_group_member_list(Qid group)->std::future<CallResult>;
	ApiCall & get_group_member_list(std::tuple<Qid> && params, const ApiRetHook<CallResult> &hook = Hook::_);)

AYAN_API(_SetFriendAddRequest)
API_DECLARE(
	[[unstable]] auto set_friend_add_request(const std::string &flag, bool approve, const std::string &remark = "")->std::future<void>;

	[[unstable]] ApiCall & set_friend_add_request(std::tuple<const std::string &, bool, const std::string &> && params);)

AYAN_API(_SetGroupAddRequest)
API_DECLARE(
	[[unstable]] auto set_group_add_request(const std::string &flag, const std::string &sub_type, bool approve, const std::string &reason = "419, I'm a bot")->std::future<void>;

	[[unstable]] ApiCall & set_group_add_request(std::tuple<const std::string &, const std::string &, bool, const std::string &> && params);)

AYAN_API(_GetSelfInfo)
API_DECLARE(
	using CallResult = std::pair<Qid, std::string>; // qq and nickname

	auto get_self_info()->std::future<CallResult>;
	ApiCall & get_self_info(const ApiRetHook<CallResult> &hook);)

// TODO
// AYAN_API(_GroupBanSingle)
// API_DECLARE(
//	auto group_ban_single(Qid group, Qid user, size_t duration = 30 * 60) -> std::future<void>;
//	ApiCall& group_ban_single(std::tuple<Qid, Qid, size_t>&& params);
//)

AYAN_API(_WordSlice)
API_DECLARE(
	auto word_slice(const std::wstring &cn_sentence)->std::future<std::vector<std::wstring>>;
	ApiCall & word_slice(std::tuple<const std::wstring &> && params, const ApiRetHook<std::vector<std::wstring>> &hook = Hook::_);)

AYAN_API(_OcrImage)
API_DECLARE(
	[[unstable]] auto ocr_image(const std::string &image_file)->std::future<OcrResult>;

	[[unstable]] ApiCall & ocr_image(std::tuple<const std::string &> && params, const ApiRetHook<OcrResult> &hook = Hook::_);)

#undef UNCOMMON_INCLUDE
#undef AYAN_API
#undef API_DECLARE