all: pre back front

pre:
	mkdir -p build/

back:
	g++ --std=c++20 -s -O2 main.cpp -o build/Snek3D -static -flto -mtune=native -DNDEBUG -Wno-narrowing

front: deps
	cd frontend && make -j 4
	cp Snek3D-Frontend ../build/

deps:
	cp frontend/vertex.vert build/
	cp frontend/frag.frag   build/
	cp frontend/ico.png     build/

clean:
	rm -r build/
