#include <cstdio>
#include <cstdio>

#include "pipette/pipe.hpp"
#include "backend/SnekGame3D.hpp"

/*
$ HERE ARE FRONTEND PROTOCOL DETAILS :-

[backend started by user]
[backend pipes the  frontend]

backend (u8 max bits in coord) >> frontend
backend (x,y,z world size (from 1,1,1 to x,y,z)) >> frontend

<repeat>
	frontend (key pressed) >> backend
	backend (u32 - no of points) >> frontend
	backend (x,y,z,x,y,z....) >> frontend
</repeat>
*/
/*
int operator >> (const pipette::pipe& pipe, char& ch)
{
	return pipe.read((uint8_t*)&ch, 1);
}

template <typename T>
void operator << (const pipette::pipe& pipe, const Point3D<T>& pnt)
{
	pipe.write((uint8_t*)&pnt.x, sizeof(T));
	pipe.write((uint8_t*)&pnt.y, sizeof(T));
	pipe.write((uint8_t*)&pnt.z, sizeof(T));
}
*/
int main()
{
	using mint = uint8_t;
	auto mint_sz = sizeof(mint);

	char key; uint32_t num_pnts;
	SnekGame3D<mint> game(16,16,16);
	
	pipette::pipe pfront;
	pfront.open("./Snek3D-Frontend - -", true);

	pfront.write((uint8_t*)(mint_sz * 8u), 1);
	
	pfront.write((uint8_t*)&game.wrld.x, mint_sz);
	pfront.write((uint8_t*)&game.wrld.y, mint_sz);
	pfront.write((uint8_t*)&game.wrld.z, mint_sz);
	
	while (true)
	{
		if (!pfront.read((uint8_t*)&key, 1))
		{
			std::puts("Err reading key");
			continue;
		}
		else
		{
			game.nextFrame(key);
		}
		
		num_pnts = game.snek.size() + 1;
		pfront.write((uint8_t*)&num_pnts, 4);

		pfront.write((uint8_t*)&game.food.x, mint_sz);
		pfront.write((uint8_t*)&game.food.y, mint_sz);
		pfront.write((uint8_t*)&game.food.z, mint_sz);
		
		for	(const auto& pnt : game.snek)
		{
			pfront.write((uint8_t*)&pnt.x, mint_sz);
			pfront.write((uint8_t*)&pnt.y, mint_sz);
			pfront.write((uint8_t*)&pnt.z, mint_sz);
		}
	}
}
