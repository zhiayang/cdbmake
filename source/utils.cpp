// utils.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

// util.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include <cctype>
#include <cstdio>
#include <cassert>
#include <cstdlib>

#include <unistd.h>

#include <span>
#include <chrono>
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>
#include <charconv>
#include <algorithm>
#include <filesystem>
#include <string_view>

#include <zst.h>
#include <zpr.h>

namespace stdfs = std::filesystem;

namespace util
{
	std::string readEntireFile(stdfs::path path)
	{
		auto f = std::ifstream(path.string(), std::ios::in | std::ios::binary);
		std::stringstream ss {};
		ss << f.rdbuf();

		return ss.str();
	}

	std::string_view trim(std::string_view sv)
	{
		while(sv.size() > 0 && sv.find_first_of(" \t") == 0)
			sv.remove_prefix(1);

		while(sv.size() > 0 && sv.find_last_of(" \t") == sv.size() - 1)
			sv.remove_suffix(1);

		return sv;
	}

	std::vector<std::string_view> splitString(std::string_view str, char delim)
	{
		std::vector<std::string_view> ret {};

		while(true)
		{
			size_t ln = str.find(delim);

			if(ln != std::string_view::npos)
			{
				ret.emplace_back(str.data(), ln);
				str.remove_prefix(ln + 1);
			}
			else
			{
				break;
			}
		}

		// account for the case when there's no trailing newline, and we still have some stuff stuck in the view.
		if(!str.empty())
			ret.emplace_back(str.data(), str.length());

		return ret;
	}
}

[[noreturn]] void zst::error_and_exit(const char* s, size_t n)
{
	zpr::fprintln(stderr, "{}", std::string_view(s, n));
	abort();
}
