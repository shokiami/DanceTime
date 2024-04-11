CXX := g++
FLAGS := -std=c++17 `pkg-config --cflags opencv4`
LIBS := -Wl,-rpath,./ mediapipe/libmediapipe.so -lrtaudio -lavcodec -lavformat -lavutil `pkg-config --libs opencv4`
ifeq ($(BUILD),RELEASE)
OPT := -O3
else
OPT := -g
endif

SOURCES := $(wildcard src/*.cc)
HEADERS := $(wildcard src/*.h)
OBJECTS := $(patsubst src/%.cc,obj/%.o,$(SOURCES))

main: $(OBJECTS) mediapipe/libmediapipe.so
	$(CXX) $(FLAGS) $(OPT) $(OBJECTS) -o main $(LDFLAGS) $(LIBS)

$(OBJECTS): obj/%.o : src/%.cc $(HEADERS)
	$(CXX) $(FLAGS) $(OPT) -c $< -o $@

.PHONY: clean setup

setup:
	mkdir obj

clean:
	rm -f main obj/*.o
	rm -f $(TARGET)
