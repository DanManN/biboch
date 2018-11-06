PRODUCT := checkers.out
num := 2
p0 := y
p1 := y

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
	rm -f $(PRODUCT) $(OBJFILES)

.PHONY: clean

sample: $(PRODUCT)
	echo -e $(p0)"\n"$(p1)"\ntestboards/sampleCheckers"$(num)".txt" | ./checkers.out

.PHONY: sample
