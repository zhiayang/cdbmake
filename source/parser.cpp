// parser.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include <cassert>

#include <filesystem>
#include <string>
#include <string_view>

#include "cdbmake.h"

namespace cdb
{
	static bool is_compiler_invocation(std::string_view str)
	{
		return str.find("clang") != std::string_view::npos || str.find("gcc") != std::string_view::npos ||
		       str.find("g++") != std::string_view::npos;
	}

	static bool is_source_file(std::string_view str)
	{
		return str.ends_with(".c") || str.ends_with(".h") || str.ends_with(".cc") || str.ends_with(".cpp") ||
		       str.ends_with(".cxx") || str.ends_with(".hh") || str.ends_with(".hpp") || str.ends_with(".hxx");
	}

	static std::string unescape_string(std::string_view s)
	{
		std::string ret {};
		ret.reserve(s.size());

		// this is kinda rudimentary and not 100% correct, but it works for my use case.
		for(size_t i = 0; i < s.size(); i++)
		{
			if(s[i] == '\\')
			{
				continue;
			}
			else
			{
				ret.push_back(s[i]);
			}
		}

		return ret;
	}

	ErrorOr<void> parseCommandOutput(Database& db, MakeState& ms, std::string_view line)
	{
		if(line.starts_with("make: Entering directory ") ||
			(line.starts_with("make[") && line.find("]: Entering directory ") != std::string_view::npos))
		{
			auto dir = line.substr(1 + line.find_first_of("'`"));
			assert(dir.back() == '\'');
			dir.remove_suffix(1);

			zpr::println("{} # {}", zpr::w(static_cast<int>(ms.dir_stack.size() * 2))(""), dir);
			ms.dir_stack.push_back(dir);
		}
		else if(line.starts_with("make: Leaving directory ") ||
				(line.starts_with("make[") && line.find("]: Leaving directory ") != std::string_view::npos))
		{
			auto dir = line.substr(1 + line.find_first_of("'`"));
			assert(dir.back() == '\'');
			dir.remove_suffix(1);

			if(ms.dir_stack.back() != dir)
				return ErrFmt("mismatched dirstack; expected '{}', got '{}'", ms.dir_stack.back().string(), dir);

			ms.dir_stack.pop_back();
		}

		auto parts = util::splitString(line, ' ');

		// check if the first space-separated token, after getting the basename,
		// contains 'gcc', 'g++', 'clang', or 'clang++'
		auto compiler = stdfs::path(parts[0]).filename().string();
		if(not is_compiler_invocation(compiler))
			return Ok();

		Command cmd {};
		cmd.args.emplace_back(parts[0]);

		bool ignore_next = false;
		for(size_t i = 1; i < parts.size(); i++)
		{
			if(parts[i].empty())
				continue;

			if(parts[i] == "-o" || parts[i] == "-include" || parts[i] == "-MF")
				ignore_next = true;

			else if(not ignore_next && is_source_file(parts[i]))
				cmd.file = parts[i];

			else if(ignore_next)
				ignore_next = false;

			cmd.args.emplace_back(unescape_string(parts[i]));
		}

		if(cmd.file.empty())
			return Ok();

		cmd.directory = ms.dir_stack.back();

		// copy out the key, because we move it
		auto key = cmd.file;
		db.files[cmd.directory / key] = std::move(cmd);

		zpr::println("{} + {}", zpr::w(static_cast<int>(ms.dir_stack.size() * 2))(""), key.string());
		return Ok();
	}
}
