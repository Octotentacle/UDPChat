#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <set>
#include "chat_message.h"

using boost::asio::ip::udp;

class udp_server
{
public:
    udp_server(boost::asio::io_service& io_service, unsigned short port)
            : socket_(io_service, udp::endpoint(udp::v4(), port)) {
        start_receive();
    }

private:
    void start_receive() {
        socket_.async_receive_from(
                boost::asio::buffer(recv_buffer_), remote_endpoint_,
                boost::bind(&udp_server::handle_receive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive(const boost::system::error_code& error,
                        std::size_t /*bytes_transferred*/) {
        if (!error || error == boost::asio::error::message_size)
        {

            ChatMessage accepted{recv_buffer_.data()};

            if (accepted.GetType() == MessageType::kOnlineState) {
                socket_.send_to(boost::asio::buffer(recv_buffer_), remote_endpoint_);
                std::cout << BOLD(FGRN("New connection from ")) << remote_endpoint_ << '\n';
                endpoins_.insert(remote_endpoint_);
            } else if (accepted.GetType() == MessageType::kSimpleMessage) {
                std::cout << FGRN("Message from ") << remote_endpoint_ << '\n';
                std::cout << accepted.ReadText() << '\n';
                for (const auto& endpoint : endpoins_) {
                    if (remote_endpoint_ == endpoint) {
                        continue;
                    }
                    socket_.async_send_to(boost::asio::buffer(accepted.GetRawData()), endpoint,
                                          boost::bind(&udp_server::handle_send, this, accepted.GetRawData(),
                                                      boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred));
                }
            } else {
                std::cout << BOLD(FRED("UNKNOWN MESSAGE")) << '\n';
                std::cout << accepted.GetRawData().data();
            }
            start_receive();
        }
    }

    void handle_send(boost::array<char, ChatMessage::kMessageSize>,
                     const boost::system::error_code& /*error*/,
                     std::size_t /*bytes_transferred*/) {
    }

    std::set<udp::endpoint> endpoins_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, ChatMessage::kMessageSize> recv_buffer_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cout << "Usage: server <port>" << std::endl;
            exit(1);
        }
        boost::asio::io_service io_service;
        udp_server server(io_service, std::atoi(argv[1]));
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}