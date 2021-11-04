bin/bloxorz: bin/File.o bin/Proof.o bin/Solver.o bin/bloxorz.o
	g++ -o $@ -std=c++11 -g $^

bin/File.o: src/sat/File.cpp
	g++ -c -std=c++11 -g $< -o $@

bin/Proof.o: src/sat/Proof.cpp
	g++ -c -std=c++11 -g $< -o $@

bin/Solver.o: src/sat/Solver.cpp
	g++ -c -std=c++11 -g $< -o $@

bin/bloxorz.o: src/main/bloxorz.cpp
	g++ -c -std=c++11 -g $< -o $@

clean:
	rm -f bin/*.o bin/bloxorz bin/tags
