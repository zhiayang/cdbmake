// make.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include <zprocpipe.h>
namespace zpp = zprocpipe;

#include "cdbmake.h"

namespace cdb
{
	static std::optional<std::string_view> consume_one_line(const std::string& str, size_t& start_idx)
	{
		auto sv = std::string_view(str).substr(start_idx);
		if(auto idx = sv.find('\n'); idx != std::string_view::npos)
		{
			start_idx += idx + 1;
			return sv.substr(0, idx);
		}
		else
		{
			return std::nullopt;
		}
	}


	ErrorOr<void> runMake(Database& db, std::vector<std::string> args)
	{
		// n: dry run (and print commands)
		// k: keep going on errors
		// w: print directory changes
		// B: force rebuild all targets
		args.push_back("-nkwB");
		args.push_back("-j1");

		zpr::println("** running: make {}", args);

		auto foo = zpp::runProcess("make", args);
		if(not foo.first.has_value())
			return Err(foo.second);

		auto make = std::move(foo.first.value());

		std::string stdout;
		std::string stderr;
		size_t stdout_skip_idx = 0;
		size_t stderr_skip_idx = 0;

		MakeState ms {};

		while(make.isAlive())
		{
			if(make.pollOutput(stdout, stderr, /* milliseconds: */ 50))
			{
				bool parsed = false;
				do
				{
					parsed = false;
					if(auto line = consume_one_line(stdout, stdout_skip_idx); line.has_value())
					{
						parsed = true;
						if(auto x = parseCommandOutput(db, ms, *line); x.is_err())
							return Err(x.error());
					}

					if(auto line = consume_one_line(stderr, stderr_skip_idx); line.has_value())
						parsed = true, zpr::fprintln(::stderr, "{}", *line);

				} while(parsed);
			}
		}

		return Ok();
	}
}
