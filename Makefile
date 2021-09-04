CXX := g++
FLAGS := `pkg-config --cflags opencv4` -std=c++17
LIBS := `pkg-config --libs opencv4` -Wl,-rpath,./ libmediapipe.so
ifeq ($(BUILD),RELEASE)
OPT := -O3
else
OPT := -g
endif

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h)
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

main: $(OBJECTS) libmediapipe.so
	$(CXX) $(FLAGS) $(OPT) $(OBJECTS) -o main $(LDFLAGS) $(LIBS)

$(OBJECTS): obj/%.o : %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(OPT) -c $< -o $@

.PHONY: clean setup

setup:
	mkdir obj

clean:
	rm -rf obj/*
	rm -f $(TARGET)
