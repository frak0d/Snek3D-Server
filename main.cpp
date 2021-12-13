#include <cstdio>
#include <concepts>

#include "pipette/pipe.cpp"
#include "backend/SnekGame3D.hpp"

/*
$ HERE ARE FRONTEND PROTOCOL DETAILS :-

[backend started by user]
[backend pipes the  frontend]

backend (u8 max bits in one coord) >> frontend
backend (x,y,z world size (from 1,1,1 to x,y,z)) >> frontend

<repeat>
	frontend (key pressed) >> backend
	backend (u32 - no of points) >> frontend
	backend (x,y,z,x,y,z....) >> frontend
</repeat>
*/

int operator >> (const pipette::pipe& pipe, std::integral auto& T)
{
	return pipe.read((uint8_t*)&T, sizeof(T));
}

int operator << (const pipette::pipe& pipe, const std::integral auto& T)
{
	return pipe.write((uint8_t*)&T, sizeof(T));
}

template <typename T>
void operator << (const pipette::pipe& pipe, const Point3D<T>& pnt)
{
	pipe << pnt.x ; pipe << pnt.y ; pipe << pnt.z;
}

int main()
{
	using mint = uint8_t;

	char key;
	int err_cnt=0;
	uint32_t num_pnts;
	
	SnekGame3D<mint> game(20,20,20);
	
	pipette::pipe pfront;
	if (!pfront.open("./Snek3D-Frontend - -", true))
	{
		std::puts("Error opening Pipe !");
	//	std::exit(-2);
	}

	pfront << (uint8_t)(sizeof(mint)*8u); // max bits per coord
	pfront << game.wrld; // max world size
	
	while (true)
	{
		if (!(pfront >> key))
		{
			++err_cnt;
			std::printf("Error Reading Key... (%-2d errors)\n", err_cnt);
			
			if (err_cnt > 20)
			{
				std::puts("\nToo Many Errors, Exiting...");
				std::exit(-3);
			}
			else continue;
		}
		
		game.nextFrame(key);
		num_pnts = game.snek.size() + 1;
		
		pfront << num_pnts;	
		pfront << game.food;
		for (const auto& piece : game.snek) pfront << piece;
	}
}
