release: backend frontend
debug: backend-dbg frontend-dbg

bdir:
	mkdir -p build/

clean:
	rm -rf build/

backend: bdir
	$(CXX) --std=c++20 -s -O2 main.cpp -o build/Snek3D -Wno-narrowing -static -flto

backend-dbg: bdir
	$(CXX) --std=c++20 -g -Og main.cpp -o build/Snek3D -Wno-narrowing

frontend: bdir
	cd frontend && $(MAKE)
	mv frontend/build/* build/

frontend-dbg: bdir
	cd frontend && $(MAKE) debug
	mv frontend/build/* build/
