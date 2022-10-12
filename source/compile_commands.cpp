// compile_commands.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include <fstream>

#include <picojson.h>
namespace pj = picojson;

#include "cdbmake.h"

namespace cdb
{
	static std::vector<std::string> split_command(std::string_view cmd)
	{
		std::vector<std::string> args {};
		args.emplace_back();

		int state = 0;
		while(cmd.size() > 0)
		{
			if(state == 0)
			{
				while(cmd.size() > 0)
				{
					if(cmd[0] == '"')
					{
						state = 1;
						break;
					}
					else if(cmd[0] != '\\')
					{
						args.back().push_back(cmd[0]);
					}
					else if(cmd.size() > 1)
					{
						args.back().push_back(cmd[1]);
						cmd.remove_prefix(1);
					}
					cmd.remove_prefix(1);
				}

				// finish the current thing.
				if(cmd.empty())
					break;

				args.emplace_back();
			}
			else if(state == 1)
			{
				assert(cmd[0] == '"');
				while(cmd.size() > 0)
				{
					if(cmd[0] == '\\' && cmd.size() > 1)
					{
						args.back().push_back(cmd[1]);
						cmd.remove_prefix(1);
					}
					else if(cmd[0] == '"')
					{
						cmd.remove_prefix(1);
						state = 0;
						break;
					}
					else
					{
						args.back().push_back(cmd[0]);
						cmd.remove_prefix(1);
					}
				}

				// finish the current thing.
				if(cmd.empty())
					break;

				args.emplace_back();
			}
		}

		return args;
	}


	static ErrorOr<Command> parse_command(Database& db, pj::object& obj)
	{
		if(obj.find("file") == obj.end())
			return ErrFmt("missing key 'file'");
		else if(obj.find("directory") == obj.end())
			return ErrFmt("missing key 'directory'");
		else if(obj.find("command") == obj.end() && obj.find("arguments") == obj.end())
			return ErrFmt("one of 'command' or 'arguments' must be given");

		Command cmd {};
		if(auto& f = obj["file"]; not f.is_str())
			return ErrFmt("expected string value for 'file' key");
		else
			cmd.file = f.as_str();

		if(auto& d = obj["directory"]; not d.is_str())
			return ErrFmt("expected string value for 'directory' key");
		else
			cmd.directory = d.as_str();

		if(auto command_str = obj.find("command"); command_str != obj.end())
		{
			if(not command_str->second.is_str())
				return ErrFmt("expected string value for 'command' key");

			cmd.args = split_command(command_str->second.as_str());
		}
		else
		{
			auto arg_list = obj["arguments"];
			if(not arg_list.is_arr())
				return ErrFmt("expected array value for 'arguments' key");

			auto& arr = arg_list.as_arr();
			for(auto& arg : arr)
			{
				if(not arg.is_str())
					return ErrFmt("expected string value in 'arguments' array");

				cmd.args.push_back(arg.as_str());
			}
		}

		return Ok(std::move(cmd));
	}

	ErrorOr<Database> parseCompileCommandsJson(const stdfs::path& json_path)
	{
		// if it doesn't exist, return a default-constructed database
		if(not stdfs::exists(json_path))
			return Ok<Database>();

		auto config_contents = util::readEntireFile(json_path);
		pj::value compile_db_json {};

		std::string err;
		pj::parse(compile_db_json, config_contents.begin(), config_contents.end(), &err);
		if(not err.empty())
			return ErrFmt("parse error: {}", err);

		if(not compile_db_json.is_arr())
			return ErrFmt("expected array at top-level");

		Database db {};
		auto& json_top = compile_db_json.as_arr();

		for(auto& file : json_top)
		{
			if(not file.is_obj())
				return ErrFmt("expected json object in top-level array");

			parse_command(db, file.as_obj());
		}

		return Ok(std::move(db));
	}

	void writeCompileCommandsJson(const Database& db, bool write_file, const stdfs::path& json)
	{
		std::vector<pj::value> cmds {};
		for(auto& [_, file] : db.files)
		{
			std::map<std::string, pj::value> obj {};
			obj["directory"] = pj::value(file.directory);
			obj["file"] = pj::value(file.file);

			std::vector<pj::value> args {};
			for(auto& arg : file.args)
				args.emplace_back(arg);

			obj["arguments"] = pj::value(std::move(args));
			cmds.push_back(pj::value(std::move(obj)));
		}

		auto top = pj::value(cmds).serialise(/* pretty: */ true);

		if(write_file)
		{
			auto out = std::ofstream(json, std::ios::out | std::ios::binary);
			out << top;
		}
		else
		{
			zpr::println("{}", top);
		}
	}
}
