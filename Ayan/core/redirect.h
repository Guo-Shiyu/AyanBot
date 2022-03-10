#pragma once

#include <iostream>

namespace ayan
{
	struct RedirectInfo
	{
		std::ostream *log, *err, *dbg;

		static RedirectInfo Default() noexcept
		{
			return RedirectInfo{
				.log = std::_Ptr_clog,
				.err = std::_Ptr_cerr,
				.dbg = std::_Ptr_cout,
			};
		}
	};
}
