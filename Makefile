# Auto-generated by init.py
CLANG ?= clang++

all: StackTraceTests ExceptionTests RefTests GeneratorTests StringTests JSONTests

StackTraceTests: build/StackTraceTests.asan build/StackTraceTests.ubsan build/StackTraceTests.tsan build/StackTraceTests
	LSAN_OPTIONS=suppressions=ignorelist.lsan.txt,report_objects=1,max_leaks=3,print_suppressions=0 ASAN_OPTIONS=detect_leaks=1 build/StackTraceTests.asan && build/StackTraceTests.ubsan && build/StackTraceTests.tsan && build/StackTraceTests

build/StackTraceTests.asan: test/StackTraceTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/StackTraceTests.asan test/StackTraceTests.cc

all_headers: src/cxx/StackTrace.h src/cxx/JSON.h src/cxx/Exception.h src/cxx/Ref.h src/cxx/Generator.h src/cxx/String.h

builddir:
	mkdir -p build

build/StackTraceTests.ubsan: test/StackTraceTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/StackTraceTests.ubsan test/StackTraceTests.cc

build/StackTraceTests.tsan: test/StackTraceTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/StackTraceTests.tsan test/StackTraceTests.cc

build/StackTraceTests: test/StackTraceTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt -o build/StackTraceTests test/StackTraceTests.cc

ExceptionTests: build/ExceptionTests.asan build/ExceptionTests.ubsan build/ExceptionTests.tsan build/ExceptionTests
	LSAN_OPTIONS=suppressions=ignorelist.lsan.txt,report_objects=1,max_leaks=3,print_suppressions=0 ASAN_OPTIONS=detect_leaks=1 build/ExceptionTests.asan && build/ExceptionTests.ubsan && build/ExceptionTests.tsan && build/ExceptionTests

build/ExceptionTests.asan: test/ExceptionTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/ExceptionTests.asan test/ExceptionTests.cc

build/ExceptionTests.ubsan: test/ExceptionTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/ExceptionTests.ubsan test/ExceptionTests.cc

build/ExceptionTests.tsan: test/ExceptionTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/ExceptionTests.tsan test/ExceptionTests.cc

build/ExceptionTests: test/ExceptionTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt -o build/ExceptionTests test/ExceptionTests.cc

RefTests: build/RefTests.asan build/RefTests.ubsan build/RefTests.tsan build/RefTests
	LSAN_OPTIONS=suppressions=ignorelist.lsan.txt,report_objects=1,max_leaks=3,print_suppressions=0 ASAN_OPTIONS=detect_leaks=1 build/RefTests.asan && build/RefTests.ubsan && build/RefTests.tsan && build/RefTests

build/RefTests.asan: test/RefTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/RefTests.asan test/RefTests.cc

build/RefTests.ubsan: test/RefTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/RefTests.ubsan test/RefTests.cc

build/RefTests.tsan: test/RefTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/RefTests.tsan test/RefTests.cc

build/RefTests: test/RefTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt -o build/RefTests test/RefTests.cc

GeneratorTests: build/GeneratorTests.asan build/GeneratorTests.ubsan build/GeneratorTests.tsan build/GeneratorTests
	LSAN_OPTIONS=suppressions=ignorelist.lsan.txt,report_objects=1,max_leaks=3,print_suppressions=0 ASAN_OPTIONS=detect_leaks=1 build/GeneratorTests.asan && build/GeneratorTests.ubsan && build/GeneratorTests.tsan && build/GeneratorTests

build/GeneratorTests.asan: test/GeneratorTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/GeneratorTests.asan test/GeneratorTests.cc

build/GeneratorTests.ubsan: test/GeneratorTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/GeneratorTests.ubsan test/GeneratorTests.cc

build/GeneratorTests.tsan: test/GeneratorTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/GeneratorTests.tsan test/GeneratorTests.cc

build/GeneratorTests: test/GeneratorTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt -o build/GeneratorTests test/GeneratorTests.cc

StringTests: build/StringTests.asan build/StringTests.ubsan build/StringTests.tsan build/StringTests
	LSAN_OPTIONS=suppressions=ignorelist.lsan.txt,report_objects=1,max_leaks=3,print_suppressions=0 ASAN_OPTIONS=detect_leaks=1 build/StringTests.asan && build/StringTests.ubsan && build/StringTests.tsan && build/StringTests

build/StringTests.asan: test/StringTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/StringTests.asan test/StringTests.cc

build/StringTests.ubsan: test/StringTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/StringTests.ubsan test/StringTests.cc

build/StringTests.tsan: test/StringTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/StringTests.tsan test/StringTests.cc

build/StringTests: test/StringTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt -o build/StringTests test/StringTests.cc

JSONTests: build/JSONTests.asan build/JSONTests.ubsan build/JSONTests.tsan build/JSONTests
	LSAN_OPTIONS=suppressions=ignorelist.lsan.txt,report_objects=1,max_leaks=3,print_suppressions=0 ASAN_OPTIONS=detect_leaks=1 build/JSONTests.asan && build/JSONTests.ubsan && build/JSONTests.tsan && build/JSONTests

build/JSONTests.asan: test/JSONTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/JSONTests.asan test/JSONTests.cc

build/JSONTests.ubsan: test/JSONTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/JSONTests.ubsan test/JSONTests.cc

build/JSONTests.tsan: test/JSONTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/JSONTests.tsan test/JSONTests.cc

build/JSONTests: test/JSONTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt -o build/JSONTests test/JSONTests.cc

clean:
	rm -rf build/

msan: build/StackTraceTests.msan build/ExceptionTests.msan build/RefTests.msan build/GeneratorTests.msan build/StringTests.msan build/JSONTests.msan
	true && build/StackTraceTests.msan && build/ExceptionTests.msan && build/RefTests.msan && build/GeneratorTests.msan && build/StringTests.msan && build/JSONTests.msan

build/StackTraceTests.msan: test/StackTraceTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=memory -o build/StackTraceTests.msan test/StackTraceTests.cc -fsanitize-ignorelist=ignorelist.msan.txt -fsanitize-memory-track-origins -Wl,-no-pie

build/ExceptionTests.msan: test/ExceptionTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=memory -o build/ExceptionTests.msan test/ExceptionTests.cc -fsanitize-ignorelist=ignorelist.msan.txt -fsanitize-memory-track-origins -Wl,-no-pie

build/RefTests.msan: test/RefTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=memory -o build/RefTests.msan test/RefTests.cc -fsanitize-ignorelist=ignorelist.msan.txt -fsanitize-memory-track-origins -Wl,-no-pie

build/GeneratorTests.msan: test/GeneratorTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=memory -o build/GeneratorTests.msan test/GeneratorTests.cc -fsanitize-ignorelist=ignorelist.msan.txt -fsanitize-memory-track-origins -Wl,-no-pie

build/StringTests.msan: test/StringTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=memory -o build/StringTests.msan test/StringTests.cc -fsanitize-ignorelist=ignorelist.msan.txt -fsanitize-memory-track-origins -Wl,-no-pie

build/JSONTests.msan: test/JSONTests.cc all_headers builddir
	$(CLANG) @compile_flags.txt @debugging_flags.txt -fsanitize=memory -o build/JSONTests.msan test/JSONTests.cc -fsanitize-ignorelist=ignorelist.msan.txt -fsanitize-memory-track-origins -Wl,-no-pie

