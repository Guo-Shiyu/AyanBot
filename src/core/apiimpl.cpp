#include "apicall.h"

/// cast api class pointer to other class pointer
#define as_warp(api_ptr) static_cast<ApiWarpper *>(api_ptr)
#define as_call(api_ptr) reinterpret_cast<ApiCall *>(as_warp(api_ptr))

template <typename T, const char *Field>
requires(std::is_fundamental<T>::value) 
auto take_field = [](const json &pkt) -> T {
	return pkt[Field].get<T>();
};

template <typename T>
auto take_full = [](const json &pkt) -> T {
	return pkt.get<T>();
};

template <typename T, template <typename I> class C>
C<T> take_all(const json &pkt)
{
	C<T> ret;
	ret.reserve(pkt.size());
	for (const auto &elem : pkt)
		ret.push_back(take_full<T>(elem));
	return ret;
}

namespace kmap
{
	constexpr const char *send_pri_msg = "send_private_msg";

	// 'static' means link stage address required
	constexpr static const char msg_id[] = "message_id";
}

auto _SendPrivateMessage::send_private_msg(Qid id, const Message &msg) -> std::future<MsgId>
{
	return as_call(this)
		->action(kmap::send_pri_msg)
		.params("user_id", id)
		.params("message", msg.as_json())
		.get<MsgId>(take_field<MsgId, kmap::msg_id>);
}

ApiCall &_SendPrivateMessage::send_private_msg(std::tuple<Qid, const Message &> &&params, const ApiRetHook<MsgId> &hook)
{
	auto call = as_call(this);
	auto &[id, msg] = params;
	call->action(kmap::send_pri_msg)
		.params("user_id", id)
		.params("message", msg.as_json())
		.handle<MsgId>(take_field<MsgId, kmap::msg_id>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *send_pub_msg = "send_group_msg";
}

auto _SendGroupMessage::send_group_msg(Qid id, const Message &msg) -> std::future<MsgId>
{
	return as_call(this)
		->action(kmap::send_pub_msg)
		.params("group_id", id)
		.params("message", msg.as_json())
		.get<MsgId>(take_field<MsgId, kmap::msg_id>);
}

ApiCall &_SendGroupMessage::send_group_msg(std::tuple<Qid, const Message &> &&params, const ApiRetHook<MsgId> &hook)
{
	auto call = as_call(this);
	auto &[id, msg] = params;
	call->action(kmap::send_pub_msg)
		.params("group_id", id)
		.params("message", msg.as_json())
		.handle<MsgId>(take_field<MsgId, kmap::msg_id>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *delete_msg = "delete_msg";
}

auto _DeleteMessage::delete_msg(MsgId id) -> std::future<void>
{
	return as_call(this)
		->action(kmap::delete_msg)
		.params("message_id", id)
		.get<void>([](const auto &_)
				   { return std::future<void>{}; });
}

ApiCall &_DeleteMessage::delete_msg(std::tuple<MsgId> &&params)
{
	auto call = as_call(this);
	auto &[msgid] = params;
	call->action(kmap::delete_msg)
		.params("message_id", msgid)
		.call();

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *get_frind_list = "get_friend_list";
}

auto _GetFriendList::get_friend_list() -> std::future<CallResult>
{
	return as_call(this)
		->action(kmap::get_frind_list)
		.get<CallResult>(take_all<FriendDescriptor, std::vector>);
}

ApiCall &_GetFriendList::get_friend_list(const ApiRetHook<CallResult> &hook)
{
	auto call = as_call(this);
	call->action(kmap::get_frind_list)
		.handle<CallResult>(take_all<FriendDescriptor, std::vector>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *delete_friend = "delete_friend";
}

auto _DeleteFriend::delete_friend(Qid fid) -> std::future<void>
{
	return as_call(this)
		->action(kmap::delete_friend)
		.params("friend_id", fid)
		.get<void>([](const auto &_)
				   { return std::future<void>{}; });
}

ApiCall &_DeleteFriend::delete_friend(std::tuple<Qid> &&params)
{
	auto call = as_call(this);
	auto &[fid] = params;
	call->action(kmap::delete_friend)
		.params("friend_id", fid)
		.call();

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *get_group_info = "get_group_info";
}

auto _GetGroupInfo::get_group_info(Qid id) -> std::future<GroupDescriptor>
{
	return as_call(this)
		->action(kmap::get_group_info)
		.params("group_id", id)
		.get<GroupDescriptor>(take_full<GroupDescriptor>);
}

ApiCall &_GetGroupInfo::get_group_info(std::tuple<Qid> &&params, const ApiRetHook<GroupDescriptor> &hook)
{
	auto call = as_call(this);
	auto &[id] = params;
	call->action(kmap::get_group_info)
		.params("group_id", id)
		.handle<GroupDescriptor>(take_full<GroupDescriptor>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *get_group_list = "get_group_list";
}

auto _GetGroupList::get_group_list() -> std::future<CallResult>
{
	return as_call(this)
		->action(kmap::get_group_list)
		.get<CallResult>(take_all<GroupDescriptor, std::vector>);
}

ApiCall &_GetGroupList::get_group_list(const ApiRetHook<CallResult> &hook)
{
	auto call = as_call(this);
	call->action(kmap::get_group_list)
		.handle<CallResult>(take_all<GroupDescriptor, std::vector>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *get_group_member_info = "get_group_member_info";
}

auto _GetGroupMemberInfo::get_group_member_info(Qid group, Qid user) -> std::future<GroupMemberDescriptor>
{
	return as_call(this)
		->action(kmap::get_group_member_info)
		.params("group_id", group)
		.params("user_id", user)
		.get<GroupMemberDescriptor>(take_full<GroupMemberDescriptor>);
}

ApiCall &_GetGroupMemberInfo::get_group_member_info(std::tuple<Qid, Qid> &&params, const ApiRetHook<GroupMemberDescriptor> &hook)
{
	auto call = as_call(this);
	auto &[group, user] = params;

	call->action(kmap::get_group_member_info)
		.params("group_id", group)
		.params("user_id", user)
		.handle<GroupMemberDescriptor>(take_full<GroupMemberDescriptor>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *get_group_member_list = "get_group_member_list";
}

auto _GetGroupMemberList::get_group_member_list(Qid group_id) -> std::future<CallResult>
{
	return as_call(this)
		->action(kmap::get_group_member_list)
		.params("group_id", group_id)
		.get<CallResult>(take_all<GroupMemberDescriptor, std::vector>);
}

ApiCall &_GetGroupMemberList::get_group_member_list(std::tuple<Qid> &&params, const ApiRetHook<CallResult> &hook)
{
	auto call = as_call(this);
	auto &[group] = params;

	call->action(kmap::get_group_member_list)
		.params("group_id", group)
		.handle<CallResult>(take_all<GroupMemberDescriptor, std::vector>, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *word_slice = ".get_word_slices";
}

std::vector<std::wstring> take_all_wstr(const json &pkt)
{
	auto &array = pkt["slices"];
	std::vector<std::wstring> ret{};
	ret.reserve(array.size());
	for (auto &elem : array)
	{
		std::wstring word = utf8_to_wstr(take_full<std::string>(elem));
		if (not word.empty())
			ret.push_back(word);
	}

	return ret;
}

auto _WordSlice::word_slice(const std::wstring &cn_sentence) -> std::future<std::vector<std::wstring>>
{
	return as_call(this)
		->action(kmap::word_slice)
		.params("content", wstr_to_utf8(cn_sentence))
		.get<std::vector<std::wstring>>(take_all_wstr);
}

ApiCall &_WordSlice::word_slice(std::tuple<const std::wstring &> &&params, const ApiRetHook<std::vector<std::wstring>> &hook)
{
	auto call = as_call(this);
	auto &[sentence] = params;

	call->action(kmap::word_slice)
		.params("content", wstr_to_utf8(sentence))
		.handle<std::vector<std::wstring>>(take_all_wstr, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *set_friend_add_request = "set_friend_add_request";
}

auto _SetFriendAddRequest::set_friend_add_request(const std::string &&flag, bool approve, const std::string &remark) -> std::future<void>
{
	auto call = as_call(this)
					->action(kmap::set_friend_add_request)
					.params("flag", flag)
					.params("approve", approve);
	if (not remark.empty())
		call.params("remark", remark);

	return call.get<void>([](auto &_)
						  { return std::future<void>{}; });
}

ApiCall &_SetFriendAddRequest::set_friend_add_request(std::tuple<const std::string &, bool, const std::string &> &&params)
{
	auto call = as_call(this);
	auto &[flag, approve, remark] = params;

	call->action(kmap::set_friend_add_request)
		.params("flag", flag)
		.params("approve", approve);
	if (not remark.empty())
		call->params("remark", remark);

	call->call();
	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *set_group_add_request = "set_group_add_request";
}

auto _SetGroupAddRequest::set_group_add_request(const std::string &flag, const std::string &sub_type, bool approve, const std::string &reason) -> std::future<void>
{
	return as_call(this)
		->action(kmap::set_group_add_request)
		.params("flag", flag)
		.params("approve", approve)
		.params("sub_type", sub_type)
		.params("reason", reason)
		.get<void>([](auto &_)
				   { return std::future<void>{}; });
}

ApiCall &_SetGroupAddRequest::set_group_add_request(std::tuple<const std::string &, const std::string &, bool, const std::string &> &&params)
{
	auto call = as_call(this);
	auto &[flag, sub_type, approve, reason] = params;

	call->action(kmap::set_group_add_request)
		.params("flag", flag)
		.params("approve", approve)
		.params("sub_type", sub_type)
		.params("reason", reason)
		.call();

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *get_self_info = "get_login_info";
}

std::pair<Qid, std::string> take_self_info(const json &data)
{
	return std::make_pair(data["user_id"].get<Qid>(), data["nickname"].get<std::string>());
}

auto _GetSelfInfo::get_self_info() -> std::future<CallResult>
{
	return as_call(this)
		->action(kmap::get_self_info)
		.get<CallResult>(take_self_info);
}

ApiCall &_GetSelfInfo::get_self_info(const ApiRetHook<CallResult> &hook)
{
	auto call = as_call(this);
	call->action(kmap::get_self_info)
		.handle<CallResult>(take_self_info, hook);

	return call->clear_packet();
}

namespace kmap
{
	constexpr const char *ocr_image = "ocr_image";
}

OcrResult take_lang_analysis(const json &pkt)
{
	OcrResult result{.language = pkt["language"].get<std::string>()};

	auto &texts = pkt["texts"];
	result.texts.reserve(texts.size());
	for (auto &text : texts)
		result.texts.push_back(text.get<TextDetection>());

	return result;
}

auto _OcrImage::ocr_image(const std::string &image_file) -> std::future<OcrResult>
{
	return as_call(this)
		->action(kmap::ocr_image)
		.params("image", image_file)
		.get<OcrResult>(take_lang_analysis);
}

ApiCall &_OcrImage::ocr_image(std::tuple<const std::string &> &&params, const ApiRetHook<OcrResult> &hook)
{
	auto call = as_call(this);
	auto &[file] = params;

	call->action(kmap::ocr_image)
		.params("image", file)
		.handle<OcrResult>(take_lang_analysis, hook);

	return call->clear_packet();
}

#undef as_call
#undef as_warp