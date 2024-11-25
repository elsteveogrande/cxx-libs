all:

test: testA testT testU  # testM doesn't work on OSX :(

testA: build/StringTests.A
	./build/StringTests.A

testM: build/StringTests.M
	./build/StringTests.M

testT: build/StringTests.T
	./build/StringTests.T

testU: build/StringTests.U
	./build/StringTests.U

build/StringTests.A: builddir test/StringTests.cc src/**/*.h
	clang++ @compile_flags.txt @debugging_flags.txt \
		-fsanitize=address \
		-o build/StringTests.A \
		test/StringTests.cc

build/StringTests.M: builddir test/StringTests.cc src/**/*.h
	clang++ @compile_flags.txt @debugging_flags.txt \
		-fsanitize=memory \
		-o build/StringTests.M \
		test/StringTests.cc

build/StringTests.T: builddir test/StringTests.cc src/**/*.h
	clang++ @compile_flags.txt @debugging_flags.txt \
		-fsanitize=thread \
		-o build/StringTests.T \
		test/StringTests.cc

build/StringTests.U: builddir test/StringTests.cc src/**/*.h
	clang++ @compile_flags.txt @debugging_flags.txt \
		-fsanitize=undefined \
		-o build/StringTests.U \
		test/StringTests.cc

builddir:
	mkdir -p build

clean:
	rm -rf build *.dSYM .DS_Store