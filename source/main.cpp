// main.cpp
// Copyright (c) 2022, zhiayang
// SPDX-License-Identifier: Apache-2.0

#include "cdbmake.h"

namespace cdb
{
	ErrorOr<void> main(std::vector<std::string> args)
	{
		auto db = TRY(parseCompileCommandsJson("compile_commands.json"));

		bool dry_run = false;
		std::vector<std::string> extra_args {};

		// parse "our" arguments
		bool ignore_rest = false;
		for(auto it = args.begin(); it != args.end();)
		{
			auto& arg = *it;

			if(not ignore_rest && (arg.starts_with("-I") || arg.starts_with("-D")))
			{
				extra_args.push_back(arg);
				it = args.erase(it);
			}
			else if(not ignore_rest && arg.starts_with("-W"))
			{
				extra_args.push_back(arg);
				it = args.erase(it);
			}
			else if(not ignore_rest && (arg == "-n" || arg == "--dry-run"))
			{
				dry_run = true;
				it = args.erase(it);
			}
			else
			{
				ignore_rest = true;
				if(arg == "--")
					it = args.erase(it);
				else
					++it;
			}
		}

		if(auto e = runMake(db, args); e.is_err())
			return Err(e.error());

		for(auto& [_, file] : db.files)
			file.args.insert(file.args.end(), extra_args.begin(), extra_args.end());

		writeCompileCommandsJson(db, not dry_run, "compile_commands.json");
		return Ok();
	}
}



int main(int argc, char** argv)
{
	std::vector<std::string> args {};
	for(int i = 1; i < argc; i++)
	{
		auto sv = std::string_view(argv[i]);
		if(sv == "-h" || sv == "--help")
		{
			zpr::println(R"(
usage: cdbmake [options...] [make options...]

Options:
    -h, --help      display this help
    -n, --dry-run   don't write out compile_commands.json (but print it out)

    -I<path>
    -D<arg>
    -W<arg>         add the corresponding argument to all files
                    in the compilation database

To prevent parsing of '-n', '-I', '-W', and '-D' options, use '--', for example
$ cdbmake -- -Iaoeu

The first unknown argument, or all arguments after '--', will be passed directly
to `make`. Note that `make` itself should not appear in the argument list.

For example, to make the target `foo`, use:
$ cdbmake -Iextra_include/ foo
				)");
			exit(0);
		}

		args.emplace_back(sv);
	}

	auto x = cdb::main(std::move(args));
	if(x.is_err())
	{
		zpr::fprintln(stderr, "error: {}", x.error());
		exit(1);
	}
}
