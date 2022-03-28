release: Snek3D-Server
debug: Snek3D-Server-Debug

bdir:
	mkdir -p build/
	mkdir -p build/tmp

clean:
	rm -rf build/

WS_SRC := ixwebsocket
WS_OBJ := build/tmp

SOURCES := $(wildcard $(WS_SRC)/*.cpp)
OBJECTS := $(patsubst $(WS_SRC)/%.cpp, $(WS_OBJ)/%.o, $(SOURCES))

$(WS_OBJ)/%.o: $(WS_SRC)/%.cpp bdir
	$(CXX) --std=c++20 -I. -O2 -c $< -o $@

Snek3D-Server: bdir $(OBJECTS)
	$(CXX) --std=c++20 -I. -O2 -s main.cpp $(OBJECTS) -o build/Snek3D-Server -Wno-narrowing -lpthread -flto -static
	rm -rf $(WS_OBJ)

Snek3D-Server-Debug: bdir $(OBJECTS)
	$(CXX) --std=c++20 -I. -Og -g main.cpp $(OBJECTS) -o build/Snek3D-Server -Wno-narrowing -lpthread -flto
	rm -rf $(WS_OBJ)
