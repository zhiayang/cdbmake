## cdbmake

A thin wrapper around `make` that generates `compile_commands.json`.

```
usage: cdbmake [options...] [make options...]

Options:
    -h, --help      display this help
    -n, --dry-run   don't write out compile_commands.json (but print it out)
    -I<path>        add an additional '-I<path>' argument to all files
                    in the compilation database
    -D<arg>         add an additional '-D<arg>' argument to all files
                    in the compilation database

To prevent parsing of '-n', '-I', and '-D' options, use '--', for example
$ cdbmake -- -Iaoeu

The first unknown argument, or all arguments after '--', will be passed directly
to `make`. Note that `make` itself should not appear in the argument list.

For example, to make the target `foo`, use:
$ cdbmake -Iextra_include/ foo
```
