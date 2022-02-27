all: $(EXEC)

CFLAGS = -Wall -std=c++11 -O3 
HEADER += hash.h datatypes.hpp util.h inputadaptor.hpp bitmap.h 
SRC += hash.c inputadaptor.cpp  bitmap.c spreadsketch.cpp virtualfilter.cpp LogLogFilter.cpp sketch_cm.cpp
SKETCHHEADER += spreadsketch.hpp mrbmp.hpp virtualfilter.hpp LogLogFilter.hpp sketch_cm.hpp
SKETCHSRC += mrbmp.cpp



LIBS= -lpcap -lm

main_ss: main_ss.cpp $(SRC) $(HEADER) $(SKETCHHEADER)   
	g++ $(CFLAGS) $(INCLUDES) -o $@ $< $(SKETCHSRC) $(SRC) $(LIBS)

main_ss_hp: main_ss.cpp $(SRC) $(HEADER) $(SKETCHHEADER)   
	g++ $(CFLAGS) -DHH $(INCLUDES) -o $@ $< $(SKETCHSRC) $(SRC) $(LIBS)

test: test.cpp $(SRC) $(HEADER) $(SKETCHHEADER)   
	g++ $(CFLAGS) -DHH $(INCLUDES) -o $@ $< $(SKETCHSRC) $(SRC) $(LIBS)

zipf: zipf_gen.cpp $(SRC) $(HEADER) $(SKETCHHEADER)   
	g++ $(CFLAGS) $(INCLUDES) -o $@ $< $(SKETCHSRC) $(SRC) $(LIBS)

src_zipf: src_skew.cpp $(SRC) $(HEADER) $(SKETCHHEADER)   
	g++ $(CFLAGS) $(INCLUDES) -o $@ $< $(SKETCHSRC) $(SRC) $(LIBS)

clean:
	rm -rf $(EXEC)
	rm -rf *log*
	rm -rf *out*




