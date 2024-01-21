# Define variables
CXX = g++
CXXFLAGS = -std=c++17 -I.# 本番用
# CXXFLAGS = -std=c++17 -pg -gdwarf-4 # プロファイル用。超遅くなる。
SOURCES = 	../common/main.cpp \
			usi.cpp \
			root.cpp \
			node.cpp \
			../common/bitboard.cpp \
			../common/movegen.cpp \
			../common/position.cpp \
			../common/zobrist.cpp \
			../common/string_ex.cpp

HEADERS = 	usi.h \
			root.h \
			node.h \
			params.h \
			../common/shogi.h \
			../common/bitboard.h \
			../common/movegen.h \
			../common/zobrist.h \
			../common/position.h \
			../common/string_ex.h

OUTFILE = ./engine.exe

compile: $(OUTFILE)

$(OUTFILE): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES)

game:
	..\PetitShogi\PetitShogi.exe

profile:
	gprof $(OUTFILE) gmon.out > ./analysis/analysis_`date +%Y%m%d%H%M`.txt

clean:
	rm -f $(OUTFILE) *.o *.exe *.out *.txt

.PHONY: compile clean
