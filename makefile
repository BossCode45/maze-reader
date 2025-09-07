OBJS_DIR := ./build
SOURCE_DIR := ./src
CXX := g++
CXXFLAGS := -std=c++23 -fsanitize=address -g
LINKFLAGS := -fsanitize=address -static-libasan -ltehimage
DEBUG_FLAGS := 
EXEC := maze-reader
SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.cpp)
SOURCE_HEADERS := $(wildcard $(SOURCE_DIR)/*.h)
OBJS := $(subst $(SOURCE_DIR),$(OBJS_DIR), $(patsubst %.cpp,%.o,$(SOURCE_FILES)))

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) $(LINKFLAGS) -o $(EXEC)


clean:
	rm $(OBJS_DIR)/*
	rm $(EXEC)

$(OBJS_DIR)/%.o : $(SOURCE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR)/main.o: $(SOURCE_DIR)/main.cpp $(SOURCE_HEADERS)
$(OBJS_DIR)/reader.o: $(SOURCE_DIR)/reader.cpp $(SOURCE_DIR)/reader.h $(SOURCE_DIR)/debug.h
$(OBJS_DIR)/image.o: $(SOURCE_DIR)/image.cpp $(SOURCE_DIR)/image.h
$(OBJS_DIR)/PNGImage.o: $(SOURCE_DIR)/PNGImage.cpp $(SOURCE_DIR)/PNGImage.h $(SOURCE_DIR)/debug.h $(SOURCE_DIR)/image.h $(SOURCE_DIR)/puff.h
$(OBJS_DIR)/zlib.o: $(SOURCE_DIR)/zlib.cpp $(SOURCE_DIR)/zlib.h
