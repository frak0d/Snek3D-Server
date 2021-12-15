#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <concepts>

#include "pipette/pipette.cpp"
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

int operator >> (const pipette::fifo& pipe, std::integral auto& T)
{
	return pipe.read(&T, sizeof(T));
}

int operator << (const pipette::fifo& pipe, const std::integral auto& T)
{
	return pipe.write(&T, sizeof(T));
}

template <typename T>
void operator << (const pipette::fifo& pipe, const Point3D<T>& pnt)
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

	pipette::pipe pfront; // more contol over child process
	pfront.open("./Snek3D-Frontend /tmp/tmp_outb /tmp/tmp_inb");

	pipette::fifo fin("/tmp/tmp_inb", 'r'); // recieve from frontend
	pipette::fifo fout("/tmp/tmp_outb", 'w'); // send to frontend
	
	/*if (!*///pfront.open("./Snek3D-Frontend /tmp/tmp_outb /tmp/tmp_inb");)
	/*{
		std::puts("Error Starting Frontend !");
		cleaner(); std::exit(-2);
	}*/

	auto cleaner = [&]()
	{
		fin.close();
		fout.close();
		pfront.close();
		remove("/tmp/tmp_inb");
		remove("/tmp/tmp_outb");
	};

	fout << (uint8_t)(sizeof(mint) * 8u); // max bits per coord
	fout << game.wrld; // max world size
	
	while (true)
	{
		if (!(fin >> key))
		{
			++err_cnt;
			std::printf("Error Reading Key... (%-2d errors)\n", err_cnt);
			
			if (err_cnt >= 20)
			{
				std::puts("\nToo Many Errors, Exiting...");
				cleaner(); std::exit(-3);
			}
			else continue;
		}
		
		game.nextFrame(key);
		num_pnts = game.snek.size() + 1;
		
		fout << num_pnts;	
		fout << game.food;
		for (const auto& piece : game.snek) fout << piece;
	}
}
