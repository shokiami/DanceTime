CXX := g++
FLAGS := `pkg-config --cflags opencv4` -std=c++17
LIBS := `pkg-config --libs opencv4` -Wl,-rpath,./ mediapipe/libmediapipe.so -lrtaudio -lavcodec -lavformat -lavutil
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
	rm -f main *.o
	rm -f $(TARGET)
