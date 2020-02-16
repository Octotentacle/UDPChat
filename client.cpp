#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "chat_message.h"

using boost::asio::ip::udp;

class ChatClient {
public:

    ChatClient(boost::asio::io_service& io_service, char* host, char* port) : socket_(io_service) {
        udp::resolver resolver(io_service);
        udp::resolver::query query(udp::v4(), host, port);
        udp::resolver::iterator endpoint = resolver.resolve(query);
        receiver_endpoint_ = *endpoint;
        boost::asio::connect(socket_, endpoint);
        handle_connect();
        start_receive();
        std::cout << "Established connection with " << receiver_endpoint_ << std::endl;
    }

    void handle_connect() {
        ChatMessage initial{std::time(nullptr)};
        socket_.send_to(boost::asio::buffer(initial.GetRawData()), receiver_endpoint_);
        socket_.receive_from(boost::asio::buffer(initial.GetRawData()), receiver_endpoint_);
    };

    void start_receive() {
        socket_.async_receive_from(boost::asio::buffer(read_msg_.GetRawData()), temp_endpoint_, boost::bind(&ChatClient::handle_receive,
                this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive(const boost::system::error_code& error,
                       std::size_t /*bytes_transferred*/) {
        if (!error || error == boost::asio::error::message_size) {
            read_msg_.Decode();
            std::cout << read_msg_.ReadText() << '\n';
        }
        start_receive();
    }

    void send(ChatMessage& msg) {
        socket_.async_send_to(boost::asio::buffer(msg.GetRawData()), receiver_endpoint_, boost::bind(&ChatClient::handle_send, this, msg,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
    }

    void handle_send(ChatMessage& msg,
                     const boost::system::error_code& /*error*/,
                     std::size_t /*bytes_transferred*/) {
        //std::cout << "Message: " + *message << '\n';
    }


private:
    udp::endpoint temp_endpoint_;
    udp::endpoint receiver_endpoint_;
    udp::socket socket_;
    ChatMessage read_msg_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>" << std::endl;
            return 1;
        }

        boost::asio::io_service io_service;
        ChatClient c{io_service, argv[1], argv[2]};
        boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
        std::string data;
        while (std::getline(std::cin, data)) {
            ChatMessage message{data};
            c.send(message);
        }
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
