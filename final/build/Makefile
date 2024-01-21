build:
	cmake -DCMAKE_PREFIX_PATH=c:libtorch ..
	cmake --build . --config Release
exec:
	./Release/shogi-engine.exe
debug:
	cmake -DCMAKE_PREFIX_PATH=c:libtorch -DCMAKE_BUILD_TYPE=Debug ..
	cmake --build . --config Debug
go:
	gdb ./Debug/shogi-engine.exe