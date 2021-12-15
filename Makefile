all: backend frontend

bdir:
	mkdir -p build/

backend: bdir
	$(CXX) --std=c++20 -s -O2 main.cpp -o build/Snek3D -static -flto -DNDEBUG -Wno-narrowing

frontend: bdir fdeps
	cd frontend && make
	mv frontend/Snek3D-Frontend build/

fdeps: bdir
	cp frontend/vertex.vert build/
	cp frontend/frag.frag   build/
	cp frontend/ico.png     build/

clean:
	rm -rf build/
