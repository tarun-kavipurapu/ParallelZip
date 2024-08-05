CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -I. -Isrc/app/compressDecompress -Isrc/app/fileHandling -Isrc/app/processes

MAIN_TARGET = compressDecompress
CRYPTION_TARGET = compressMain

MAIN_SRC = main.cpp \
           src/app/processes/ProcessManagment.cpp \
           src/app/fileHandling/IO.cpp \
           src/app/fileHandling/ReadEnv.cpp \
           src/app/compressDecompress/Compress.cpp

CRYPTION_SRC = src/app/compressDecompress/CompressionMain.cpp \
               src/app/compressDecompress/Compress.cpp \
               src/app/fileHandling/IO.cpp \
               src/app/fileHandling/ReadEnv.cpp

MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
CRYPTION_OBJ = $(CRYPTION_SRC:.cpp=.o)

all: $(MAIN_TARGET) $(CRYPTION_TARGET)

$(MAIN_TARGET): $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lz

$(CRYPTION_TARGET): $(CRYPTION_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lz

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(MAIN_OBJ) $(CRYPTION_OBJ) $(MAIN_TARGET) $(CRYPTION_TARGET)

.PHONY: clean all
