#!/usr/bin/env python3

# Initialize Makefile and some skeleton test code.
# Generally this is run once (and after a header is added),
# and the resulting changes are checked in, so you don't need to run this
# unless you're hacking on this project.

from dataclasses import dataclass
import glob


# All cxx-lib headers.  (The project only provides headers!)
# These are, roughly, topologically-sorted; least-dependent to most-dependent.
# This way we can test smaller / simpler components, make sure they function,
# and then work our way up to the bigger / more-dependent parts.
headers = [
    "Util.h",
    "StackTrace.h",
    "Exception.h",
    "Ref.h",
    "Generator.h",
    "String.h",
    "JSON.h",
]


@dataclass
class Target:
    name: str           # RefTests.ubsan
    deps: list[str]     # RefTests.cc **/*.h builddir
    build: str = ""     # clang++ [...] -sanitize=undefined -o RefTests.ubsan -c test/RefTests.cc

    def print(self, targets: dict[str, "Target"], visited_names: set[str]):
        """Print self in Makefile rule format."""

        if visited_names is None:
            visited_names = set()
        if self.name in visited_names:
            return

        visited_names.add(self.name)

        line = self.name
        line += ":"
        for dep in self.deps:
            line += " " + dep
        print(line)
        if self.build:
            print("\t" + self.build)
        print()

        for dep in self.deps:
            if dep in targets:
                targets[dep].print(targets, visited_names)


class Makefile:
    def __init__(self) -> None:
        self.targets: dict[str, Target] = {}
        self.roots = [
            # "Apex" target, this is built by just running `make`
            self.add(Target("all", deps=[])),
            self.add(Target("clean", [], "rm -rf build/"))
            # Other root targets added below as well
        ]
    
    def add(self, target: Target) -> Target:
        assert target.name not in self.targets
        self.targets[target.name] = target
        return target

    def target(self, name: str) -> Target:
        return self.targets[name]

    def build(self) -> Target:
        # All tests depend on these
        builddir_target = self.add(Target("builddir", deps=[], build="mkdir -p build"))

        # All headers are to be listed as dependencies;
        # not just public ones, but the `detail` headers also.
        globbed_headers = glob.glob("src/**/*.h")
        all_headers = self.add(Target("all_headers", deps=globbed_headers))

        def make_test_target_opt(test_prog: str, test_cc: str) -> Target:
            return Target(
                name=test_prog,
                deps=[test_cc] + [all_headers.name] + [builddir_target.name],
                build=f"$(CLANG) @compile_flags.txt -o {test_prog} {test_cc}")

        def make_test_target_san(test_prog: str, test_cc: str, suf: str, san: str) -> Target:
            deps = [test_cc] + [all_headers.name] + [builddir_target.name]
            build = f"$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize={san} -o {test_prog} {test_cc}"
            if suf == "msan":
                build += " -fsanitize-ignorelist=ignorelist.msan.txt"
            return Target(name=test_prog, deps=deps, build=build)

        def run_cmd(test_prog: str, suf: str) -> str:
            cmd = f"{test_prog}.{suf}"
            if suf == "asan":
                cmd = f"ASAN_OPTIONS=detect_leaks=1 {cmd}"
                cmd = f"LSAN_OPTIONS=suppressions=ignorelist.lsan.txt {cmd}"
            return cmd

        for header in headers:
            base_name = header.rstrip(".h")
            test_cc = f"test/{base_name}Tests.cc"

            # asan, ubsan, etc. tests for this one test
            group: list[Target] = []
            test_cmds: list[str] = []

            test_prog = f"build/{base_name}Tests"
            for (suf, san) in (("asan", "address"), ("ubsan", "undefined"), ("tsan", "thread")):
                group.append(self.add(make_test_target_san(test_prog + "." + suf, test_cc, suf, san)))
                test_cmds.append(run_cmd(test_prog, suf))

            # finally add the no-SAN, optimized test
            group.append(self.add(make_test_target_opt(test_prog, test_cc)))
            test_cmds.append(test_prog)

            # add one target for all flavors of this test
            test_target = self.add(
                Target(
                    name=f"{base_name}Tests",
                    deps=[t.name for t in group],
                    build=(" && ".join(test_cmds))))

            # add under "all" target
            self.target("all").deps.append(test_target.name)

        # Add MSAN tests as a separate thing, since OSX doesn't support it.
        for (suf, san) in (("msan", "memory"),):
            san_target = self.add(Target(name=suf, deps=[], build="true"))
            self.roots.append(san_target)
            for header in headers:
                base_name = header.rstrip(".h")
                test_cc = f"test/{base_name}Tests.cc"
                prog_base = f"build/{base_name}Tests"
                san_prog = f"build/{base_name}Tests.{suf}"
                san_test = self.add(make_test_target_san(san_prog, test_cc, suf, san))
                self.targets[suf].deps.append(san_test.name)
                self.targets[suf].build += " && " + run_cmd(prog_base, suf)

        return self
    
    def print(self) -> None:
        print("# Auto-generated by init.py")
        print("CLANG ?= clang++")
        print()
        visited_names: set[str] = set()
        for root in self.roots:
            root.print(self.targets, visited_names)


if __name__ == "__main__":
    Makefile().build().print()
