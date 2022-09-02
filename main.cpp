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

[backend starts a WebSocket Server]
[frontend connects to the backend Server]

# NOTE: R,G,B are always u8
# NOTE: <-- --> indicates start and end of msg

<--
backend (u8 - max bits in one coord eg. z or x)  >> frontend
backend (x,y,z world size (from 1,1,1 to x,y,z)) >> frontend
backend (R,G,B world background color scheme)    >> frontend
-->

<repeat>
    frontend (char - key pressed) >> backend
    <--
    backend  (u32 - num_points)   >> frontend
    if [num_points == 0] # check this in frontend
    {
    	# Game Over !
    	backend (u32 - score) >> frontend;
    	# Close Connection !
    }
    backend (x,y,z,R,G,B....) >> frontend
    -->
</repeat>
*/

struct msgbuf
{
    std::string buf;
    ix::WebSocket& ws;

    msgbuf(ix::WebSocket& web_socket) : ws{web_socket} {}

    bool send()
    {
        bool sent = ws.sendBinary(buf).success;
        if (sent) buf.clear();
        return sent;
    }
};

msgbuf& operator<< (msgbuf& msgb, const std::integral auto& num)
{
    std::string data((char*)&num, (char*)(&num + 1));
    if constexpr (std::endian::native == std::endian::little)
    {
    	std::reverse(data.begin(), data.end());
    }
    msgb.buf += data;
    return msgb;
}

msgbuf& operator<< (msgbuf& msgb, const Color& color)
{
    return msgb << color.r << color.g << color.b;
}

template <typename T>
msgbuf& operator<< (msgbuf& msgb, const Point3D<T>& pnt)
{
    return msgb << pnt.x << pnt.y << pnt.z << pnt.color;
}

// SIGNAL HANDLERS //

void interrupt_handler(int)
{
    std::puts("\n\x1b[31;1m => Interrrupt Recieved, Exiting...\x1b[0m\n");
    std::exit(-1);
}

void segfault_handler(int)
{
    std::puts("\n\x1b[31;1m => Segmentation Fault, Exiting...\x1b[0m\n");
    std::exit(-4);
}

int main()
{
    std::signal(SIGINT, interrupt_handler);
    std::signal(SIGSEGV, segfault_handler);

    char key;
    using mint = uint8_t;

    SnekGame3D<mint> game(16, 16, 16);
    ix::WebSocketServer server(6969);

    server.setOnClientMessageCallback([&](std::shared_ptr<ix::ConnectionState> connectionState,
                                      ix::WebSocket& frontend, const ix::WebSocketMessagePtr& msg) -> void
    {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::clog << "Connected to" << '\n'
                      << "id: " << connectionState->getId() << '\n'
                      << "ip: " << connectionState->getRemoteIp() << '\n'
                      << "Uri: " << msg->openInfo.uri << std::endl;
            
            msgbuf msgb{frontend};
            msgb << uint8_t(sizeof(mint)*8); // bits per coordinate
            msgb << game.wrld;         // max world size & bg color
            msgb.send();
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
                std::exit(0);
            }
            else
            {
                msgbuf msgb{frontend};
                msgb << game.snek.size() + 1/*food*/;
                msgb << game.food;
                for (const auto &piece : game.snek) msgb << piece;
                msgb.send();
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
