all: backend frontend

bdir:
	mkdir -p build/

backend: bdir
	g++ --std=c++20 -s -O2 main.cpp -o build/Snek3D -static -flto -mtune=native -DNDEBUG -Wno-narrowing

frontend: bdir fdeps
	cd frontend && make -j 4
	cp Snek3D-Frontend ../build/

fdeps: bdir
	cp frontend/vertex.vert build/
	cp frontend/frag.frag   build/
	cp frontend/ico.png     build/

clean:
	rm -rf build/