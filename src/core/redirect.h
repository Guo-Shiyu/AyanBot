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
				.log = &std::cout,
				.err = &std::cerr,
				.dbg = &std::cout,
			};
		}
	};
}
