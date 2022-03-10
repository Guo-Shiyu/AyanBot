#pragma once

#include <source_location>
#include <format>
#include <exception>
#include <ios>
#include <chrono>

namespace std
{
	ostream &operator<<(ostream &os, const u8string &str);

	std::u8string to_u8str(const std::string &src);

	std::string to_string(const std::u8string &src);
}

namespace ayan
{
	void block_here() noexcept;

	std::wstring utf8_to_wstr(const std::string &src);

	std::string wstr_to_utf8(const std::wstring &src);

	std::string_view time_now();

	std::string url_encode(const std::string& utf8);

	std::string srcinfo(std::source_location &&_ = std::source_location::current());

	template <class _Rep, class _Period>
	auto count_ms(const std::chrono::duration<_Rep, _Period>& real_time) 
	{
		return std::chrono::duration_cast<decltype(1ms)>(real_time).count();
	}

	template <typename... T>
	int accumulate_ms(T&&... real_time)
	{
		return ((count_ms(real_time)) + ...);
	}

	template <typename... Args>
	std::string _debug(std::string_view fmt, std::source_location &&loc, Args &&...args)
	{
		static const char *debug_fmt = "Ayan-Debug-Info: {0} at: {1}";
		return std::format(debug_fmt, std::format(fmt, args...), srcinfo(std::forward<std::source_location>(loc)));
	}

	template <typename Except>
	void _exception_report(std::string_view err_name, std::string_view srcinfo, const char *custom_fmt = nullptr)
	{
		static const char *report_fmt = "Ayan-Runtime-Error: {0} at: {1}";
		const char *fmt = custom_fmt == nullptr ? report_fmt : custom_fmt;
		throw Except(std::format(fmt, err_name, srcinfo));
	}

	template <typename Except>
	void _exception_polish(std::string_view prog, std::string_view appendix, std::string_view err_what)
	{
		static const char *polish_fmt = "{0}\n - {1}: {2}";
		throw Except(std::format(polish_fmt, err_what, prog, appendix));
	}

	template <typename Except, typename Ret = void>
	Ret err_polish(const Except &exception, std::string_view prog, std::string_view appendix)
	{
		ayan::_exception_polish<decltype(exception)>(prog, appendix, exception.what());
		return Ret();
	}

	template <typename Except, typename Ret = void>
	Ret err_report(std::string_view name, std::source_location &&_ = std::source_location::current())
	{
		_exception_report<Except>(name, srcinfo(std::forward<std::source_location>(_)));
		return Ret();
	}

	template <typename Except, typename Ret = void>
	Ret extern_err_report(std::string_view name, std::string_view reason, std::source_location &&_ = std::source_location::current())
	{
		try
		{
			_exception_report<Except>("Extern Library Exception", srcinfo(std::forward<std::source_location>(_)), "{0} at: {1}");
		}
		catch (const Except &e)
		{
			err_polish(e, name, reason);
		}
		return Ret();
	}

	template <typename R = void>
	R unreachable()
	{
		return err_report<std::logic_error, R>("Unreachable Code");
	}

	template <typename R = void>
	R unsupported(std::string_view feature)
	{
		return err_report<std::logic_error, R>(std::format("Unsupported Feature: {}", feature));
	}
}

#define debug(fmt, ...) \
	ayan::_debug(fmt, std::source_location::current(), __VA_ARGS__)
