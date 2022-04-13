#pragma once

#ifdef _MSC_VER

#include <source_location>
#include <format>
#define fmt  std

#else

#include "fmt/core.h"

#include <cstdint>

namespace std
{
	struct source_location
	{
	public:
#if not defined(__apple_build_version__) and defined(__clang__) and (__clang_major__ >= 9)
		static constexpr source_location current(const char *fileName = __builtin_FILE(),
												 const char *functionName = __builtin_FUNCTION(),
												 const uint_least32_t lineNumber = __builtin_LINE(),
												 const uint_least32_t columnOffset = __builtin_COLUMN()) noexcept
#elif defined(__GNUC__) and (__GNUC__ > 4 or (__GNUC__ == 4 and __GNUC_MINOR__ >= 8))
		static constexpr source_location current(const char *fileName = __builtin_FILE(),
												 const char *functionName = __builtin_FUNCTION(),
												 const uint_least32_t lineNumber = __builtin_LINE(),
												 const uint_least32_t columnOffset = 0) noexcept
#else
		static constexpr source_location current(const char *fileName = "unsupported",
												 const char *functionName = "unsupported",
												 const uint_least32_t lineNumber = 0,
												 const uint_least32_t columnOffset = 0) noexcept
#endif
		{
			return source_location(fileName, functionName, lineNumber, columnOffset);
		}

		source_location(const source_location &) = default;
		source_location(source_location &&) = default;

		constexpr const char *file_name() const noexcept
		{
			return fileName;
		}

		constexpr const char *function_name() const noexcept
		{
			return functionName;
		}

		constexpr uint_least32_t line() const noexcept
		{
			return lineNumber;
		}

		constexpr std::uint_least32_t column() const noexcept
		{
			return columnOffset;
		}

	private:
		constexpr source_location(const char *fileName, const char *functionName, const uint_least32_t lineNumber,
								  const uint_least32_t columnOffset) noexcept
			: fileName(fileName), functionName(functionName), lineNumber(lineNumber), columnOffset(columnOffset)
		{
		}

		const char *fileName;
		const char *functionName;
		const std::uint_least32_t lineNumber;
		const std::uint_least32_t columnOffset;
	};
} // namespace std

#endif

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
	void clear_screen() noexcept;

	void block_here() noexcept;

	std::wstring utf8_to_wstr(const std::string &src);

	std::string wstr_to_utf8(const std::wstring &src);

	std::string_view time_now();

	std::string url_encode(const std::string &utf8);

	std::string srcinfo(std::source_location &&_ = std::source_location::current());

	using namespace std::chrono_literals;

	template <class _Rep, class _Period>
	auto count_ms(const std::chrono::duration<_Rep, _Period> &real_time)
	{
		return std::chrono::duration_cast<decltype(1ms)>(real_time).count();
	}

	template <typename... T>
	int accumulate_ms(T &&...real_time)
	{
		return ((count_ms(real_time)) + ...);
	}

	template <typename... Args>
	std::string _debug(std::string_view fmt, std::source_location &&loc, Args &&...args)
	{
		static const char *debug_fmt = "Ayan-Debug-Info: {0} at: {1}";
		return fmt::format(debug_fmt, fmt::format(fmt, args...), srcinfo(std::forward<std::source_location>(loc)));
	}

	template <typename Except>
	void _exception_report(std::string_view err_name, std::string_view srcinfo, const char *custom_fmt = nullptr)
	{
		constexpr const char *report_fmt = "Ayan-Runtime-Error: {0} at: {1}";
		std::string_view fmt = custom_fmt == nullptr ? report_fmt : custom_fmt;
		std::string info = fmt::format(report_fmt, err_name, srcinfo);
		throw Except(info);
	}

	template <typename Except>
	void _exception_polish(std::string_view prog, std::string_view appendix, std::string_view err_what)
	{
		constexpr static const char *polish_fmt = "{0}\n - {1}: {2}";
		std::string info = fmt::format(polish_fmt, err_what, prog, appendix);
		throw Except(info);
	}

	template <typename Except, typename Ret = void>
	Ret err_polish(const Except &exception, std::string_view prog, std::string_view appendix)
	{
		ayan::_exception_polish<decltype(exception)>(prog, appendix, exception.what());
		return Ret();
	}

	template <typename Except, typename Ret = void>
	Ret err_report(std::string_view name, std::source_location&& _ = std::source_location::current())
	{
		_exception_report<Except>(name, srcinfo(std::forward<std::source_location>(_)), nullptr);
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
		err_report<std::logic_error, R>(fmt::format("Unsupported Feature: {}", feature));
		return R();
	}
}

#define debug(fmt, ...) \
	ayan::_debug(fmt, std::source_location::current(), __VA_ARGS__)
