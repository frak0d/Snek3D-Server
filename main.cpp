#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <concepts>
#include <iostream>
#include <algorithm>

#include <backend/SnekGame3D.hpp>
#include <ixwebsocket/IXWebSocketServer.h>

/*
$ HERE ARE FRONTEND PROTOCOL DETAILS :-

[backend starts a WebSocket Server on localhost]
[frontend connects to the backend Server]

# NOTE: R,G,B are always u8
 
backend (u8 - max bytes in one coord eg. z)      >> frontend
backend (x,y,z world size (from 1,1,1 to x,y,z)) >> frontend
backend (R,G,B world background color scheme)    >> frontend

<repeat>
    frontend (char - key pressed) >> backend
    backend  (u32 - num_points)   >> frontend
    if [num_points == 0]
    {
    	# Game Over !
    	backend (u32 - score) >> frontend;
    	# Close Connection !
    }
    backend (x,y,z,R,G,B....) >> frontend
</repeat>
*/

bool operator<<(ix::WebSocket& ws, const std::integral auto& num)
{
    std::string data((char*)&num, (char*)(&num + 1));
    if constexpr (std::endian::native == std::endian::little)
    {
        // Convert to network BO (big endian)
    	std::reverse(data.begin(), data.end());
    }
    return ws.sendBinary(data).success;
}

template <typename T>
void operator<<(ix::WebSocket& ws, const Point3D<T>& pnt)
{
    ws << pnt.x;
    ws << pnt.y;
    ws << pnt.z;
    
    ws << pnt.color.r;
    ws << pnt.color.g;
    ws << pnt.color.b;
}

// SIGNAL HANDLERS //

void interrupt_handler(int signal)
{
    std::puts("\n\x1b[31;1m => Interrrupt Recieved, Exiting...\x1b[0m\n");
    std::exit(-1);
}

void segfault_handler(int signal)
{
    std::puts("\n\x1b[31;1m => Segmentation Fault, Exiting...\x1b[0m\n");
    std::exit(-4);
}

int main()
{
    std::signal(SIGINT, interrupt_handler);
    std::signal(SIGSEGV, segfault_handler);

    char key;
    uint32_t num_pnts;
    using mint = uint8_t;

    SnekGame3D<mint> game(16, 16, 16);
    ix::WebSocketServer server(6969);

    server.setOnClientMessageCallback([&](std::shared_ptr<ix::ConnectionState> connectionState,
                                          ix::WebSocket& frontend, const ix::WebSocketMessagePtr& msg)
    {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::clog << "Connected to" << '\n'
                      << "id: " << connectionState->getId() << '\n'
                      << "ip: " << connectionState->getRemoteIp() << '\n'
                      << "Uri: " << msg->openInfo.uri << std::endl;
            
            frontend << sizeof(mint); // bytes per cooord
            frontend << game.wrld; // max world size
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::clog << "Disconnected from " << connectionState->getRemoteIp() << '\n';
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            key = msg->str[0];
            std::clog << "Received Key: " << key << '\n';

            if (key == 'E' || !game.nextFrame(key))
            {
                std::puts("\x1b[31;1m ---=== Game Over! ===--- \x1b[0m");
                server.stop();
                return 0;
            }
            else
            {
                num_pnts = game.snek.size() + 1/*food*/;
                frontend << num_pnts;
                frontend << game.food;
                for (const auto &piece : game.snek) frontend << piece;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::clog << "Connection  Error: " << msg->errorInfo.reason << '\n'
                      << "Retries: " << msg->errorInfo.retries << '\n'
                      << "Wait  time(ms): " << msg->errorInfo.wait_time << '\n'
                      << "HTTP  Status: " << msg->errorInfo.http_status << '\n';
        }
        else if (msg->type == ix::WebSocketMessageType::Ping)
        {
            std::clog << "Received ping from " << connectionState->getRemoteIp() << '\n';
        }
        else
        {
            std::clog << "\n\x1b[33;3m <!> Internal Error: Invalid WebSocketMessageType\x1b[0m\n";
        }
    });

    if (!server.listen().first)
    {
        std::puts("\n\x1b[31;1m => Error Initiating WebSocket Server, Exiting...\x1b[0m\n");
        std::exit(-2);
    }

    server.start();
    server.wait();
}

/*
 *  EXIT CODES :-
 *  0 -> Normal Exit or Game Over
 * -1 -> Keyboard/System Interrupt
 * -2 -> Error Starting Websocket Server
 * -3 -> <RESERVED>
 * -4 -> Segmentation Fault
 */
