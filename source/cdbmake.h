// cdbmake.h
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdlib>
#include <cstdint>
#include <cstddef>

#include <zpr.h>
#include <zst.h>

#include <span>
#include <string>
#include <vector>
#include <filesystem>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

namespace stdfs = std::filesystem;

namespace util
{
	// https://en.cppreference.com/w/cpp/container/unordered_map/find
	// stupid language
	struct hasher
	{
		using is_transparent = void;
		using H = std::hash<std::string_view>;

		size_t operator()(const char* str) const
		{
			return H {}(str);
		}
		size_t operator()(std::string_view str) const
		{
			return H {}(str);
		}
		size_t operator()(const std::string& str) const
		{
			return H {}(str);
		}

		template <typename A, typename B>
		size_t operator()(const std::pair<A, B>& p) const
		{
			return std::hash<A> {}(p.first) ^ std::hash<B> {}(p.second);
		}
	};

	template <typename K, typename V>
	using hashmap = std::unordered_map<K, V, hasher, std::equal_to<>>;

	template <typename T>
	using hashset = std::unordered_set<T, hasher, std::equal_to<>>;

	std::string readEntireFile(stdfs::path path);
	std::string_view trim(std::string_view sv);
	std::vector<std::string_view> splitString(std::string_view str, char delim);
}

namespace cdb
{
	using zst::Ok;
	using zst::Err;
	using zst::ErrFmt;

	using zst::Result;

	template <typename T>
	using ErrorOr = Result<T, std::string>;

	struct Command
	{
		stdfs::path file;
		stdfs::path directory;

		std::vector<std::string> args;
	};

	struct Database
	{
		util::hashmap<std::string, Command> files;
	};

	ErrorOr<Database> parseCompileCommandsJson(const stdfs::path& json);

	struct MakeState
	{
		std::vector<stdfs::path> dir_stack;
	};

	ErrorOr<void> runMake(Database& db, std::vector<std::string> args, bool wet_run);
	ErrorOr<void> parseCommandOutput(Database& db, MakeState& ms, std::string_view line);

	void writeCompileCommandsJson(const Database& db, bool write_file, const stdfs::path& json);
}
