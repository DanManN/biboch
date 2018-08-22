PRODUCT := checkers.out

CXX := g++
LINKER := g++
CXXFLAGS := -std=c++11 -Wall -Wextra

SRCFILES := $(wildcard *.cpp) 
OBJFILES := $(patsubst %.cpp,%.o,$(SRCFILES))

$(PRODUCT): $(OBJFILES)
	$(LINKER) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c $< -o $@

clean:
	rm -f checkers.out $(OBJFILES)

.PHONY: clean
