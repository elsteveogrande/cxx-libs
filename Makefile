# Auto-generated by init.py
all: UtilTests StackTraceTests ExceptionTests RefTests GeneratorTests StringTests JSONTests

UtilTests: build/UtilTests.asan build/UtilTests.ubsan build/UtilTests.tsan
	build/UtilTests.asan && build/UtilTests.ubsan && build/UtilTests.tsan

build/UtilTests.asan: test/UtilTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/UtilTests.asan test/UtilTests.cc

all_headers: src/cxx/StackTrace.h src/cxx/JSON.h src/cxx/Util.h src/cxx/Exception.h src/cxx/Ref.h src/cxx/Generator.h src/cxx/String.h

builddir:
	mkdir -p build

build/UtilTests.ubsan: test/UtilTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/UtilTests.ubsan test/UtilTests.cc

build/UtilTests.tsan: test/UtilTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/UtilTests.tsan test/UtilTests.cc

StackTraceTests: build/StackTraceTests.asan build/StackTraceTests.ubsan build/StackTraceTests.tsan
	build/StackTraceTests.asan && build/StackTraceTests.ubsan && build/StackTraceTests.tsan

build/StackTraceTests.asan: test/StackTraceTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/StackTraceTests.asan test/StackTraceTests.cc

build/StackTraceTests.ubsan: test/StackTraceTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/StackTraceTests.ubsan test/StackTraceTests.cc

build/StackTraceTests.tsan: test/StackTraceTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/StackTraceTests.tsan test/StackTraceTests.cc

ExceptionTests: build/ExceptionTests.asan build/ExceptionTests.ubsan build/ExceptionTests.tsan
	build/ExceptionTests.asan && build/ExceptionTests.ubsan && build/ExceptionTests.tsan

build/ExceptionTests.asan: test/ExceptionTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/ExceptionTests.asan test/ExceptionTests.cc

build/ExceptionTests.ubsan: test/ExceptionTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/ExceptionTests.ubsan test/ExceptionTests.cc

build/ExceptionTests.tsan: test/ExceptionTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/ExceptionTests.tsan test/ExceptionTests.cc

RefTests: build/RefTests.asan build/RefTests.ubsan build/RefTests.tsan
	build/RefTests.asan && build/RefTests.ubsan && build/RefTests.tsan

build/RefTests.asan: test/RefTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/RefTests.asan test/RefTests.cc

build/RefTests.ubsan: test/RefTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/RefTests.ubsan test/RefTests.cc

build/RefTests.tsan: test/RefTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/RefTests.tsan test/RefTests.cc

GeneratorTests: build/GeneratorTests.asan build/GeneratorTests.ubsan build/GeneratorTests.tsan
	build/GeneratorTests.asan && build/GeneratorTests.ubsan && build/GeneratorTests.tsan

build/GeneratorTests.asan: test/GeneratorTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/GeneratorTests.asan test/GeneratorTests.cc

build/GeneratorTests.ubsan: test/GeneratorTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/GeneratorTests.ubsan test/GeneratorTests.cc

build/GeneratorTests.tsan: test/GeneratorTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/GeneratorTests.tsan test/GeneratorTests.cc

StringTests: build/StringTests.asan build/StringTests.ubsan build/StringTests.tsan
	build/StringTests.asan && build/StringTests.ubsan && build/StringTests.tsan

build/StringTests.asan: test/StringTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/StringTests.asan test/StringTests.cc

build/StringTests.ubsan: test/StringTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/StringTests.ubsan test/StringTests.cc

build/StringTests.tsan: test/StringTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/StringTests.tsan test/StringTests.cc

JSONTests: build/JSONTests.asan build/JSONTests.ubsan build/JSONTests.tsan
	build/JSONTests.asan && build/JSONTests.ubsan && build/JSONTests.tsan

build/JSONTests.asan: test/JSONTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=address -o build/JSONTests.asan test/JSONTests.cc

build/JSONTests.ubsan: test/JSONTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=undefined -o build/JSONTests.ubsan test/JSONTests.cc

build/JSONTests.tsan: test/JSONTests.cc all_headers builddir
	clang++ @compile_flags.txt @debugging_flags.txt -fsanitize=thread -o build/JSONTests.tsan test/JSONTests.cc

clean:
	rm -rf build/

