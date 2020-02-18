#ifndef CLIENT_CHAT_MESSAGE_H
#define CLIENT_CHAT_MESSAGE_H


#include <boost/array.hpp>
#include <cstring>

enum class MessageType {
    kOnlineState = 0,
    kSimpleMessage,
    kPrivateMessage,
    kUnknown = 100,
};

std::ostream& operator<<(std::ostream& os, MessageType type) {
    switch (type) {
        case MessageType::kOnlineState: os << "Client connection message"; break;
        case MessageType::kSimpleMessage: os << "Simple text message"; break;
        case MessageType::kPrivateMessage: os << "Private text message"; break;
        default: os << "Unknown message";
    }
    return os;
}

class ChatMessage {
public:

    static constexpr size_t kHeaderSize = 256;
    static constexpr size_t kMessageSize = 1025;

    ChatMessage() : data_(), type_(MessageType::kUnknown) {
        data_[0] = static_cast<char>(MessageType::kUnknown);
    }

    explicit ChatMessage(std::time_t time) : data_(), type_(MessageType::kOnlineState) {
        WriteToMessage(time);
    }

    explicit ChatMessage(char data[1025]) : data_() , type_(MessageType::kUnknown) {
        data_[0] = static_cast<char>(MessageType::kUnknown);
        memcpy(data_.data(), data, sizeof(char) * 1025);
        Decode();
    }

    explicit ChatMessage(const std::string& textMessage) : data_(), type_(MessageType::kSimpleMessage) {
        data_[0] = static_cast<char>(MessageType::kSimpleMessage);
        for (size_t i = 0; i < std::max(kMessageSize - 1, textMessage.size()); ++i) {
            data_[i + 1] = textMessage[i];
        }
    }

    void Decode() {
        switch (data_[0]) {
            case 0: {
                type_ = MessageType::kOnlineState;
                return;
            }
            case 1: {
                type_ = MessageType::kSimpleMessage;
                return;
            }
            case 2: {
                type_ = MessageType::kPrivateMessage;
                return;
            }
            default: {
                type_ = MessageType::kUnknown;
                return;
            }
        }
    }

    void MakePrivate() {
        if (type_ == MessageType::kPrivateMessage) {
            return;
        }
        if (type_ != MessageType::kSimpleMessage) {
            throw;
        }
        type_ = MessageType::kPrivateMessage;
    }

    MessageType GetType() const {
        return type_;
    }

    boost::array<char, kMessageSize>& GetRawData() {
        return data_;
    }

    boost::array<char, kMessageSize> GetRawData() const {
        return data_;
    }

    template <typename T>
    void WriteToMessage(const T& data) {
        const char* begin = reinterpret_cast<const char*>(std::addressof(data));
        const char* end = begin + sizeof(T);
        std::copy(begin, end, data_.begin() + 1);
    }

    void WriteToMessage(const std::string& data, bool isPrivate = false) {
        type_ = isPrivate ? MessageType::kPrivateMessage : MessageType::kSimpleMessage;
        data_[0] = static_cast<char>(type_);
        for (size_t i = 0; i < std::max(kMessageSize - 1, data.size()); ++i) {
            data_[i + 1] = data[i];
        }
    }

    template <typename T>
    T ReadFromMessage() const {
        T ret;
        char* begin = reinterpret_cast<char*>(std::addressof(ret));
        std::copy(data_.begin() + 1, data_.begin() + 1 + sizeof(T), begin);
        return ret;
    }

    std::string ReadText() const {
        if (type_ != MessageType::kSimpleMessage || type_ != MessageType::kPrivateMessage) {
            throw;
        }
        std::string ret{};
        for (size_t i = 1; i < kMessageSize && data_[i] != 0; ++i) {
            ret.push_back(data_[i]);
        }
        return ret;
    }


private:
    MessageType type_;
    boost::array<char, kMessageSize> data_;

};


#endif //CLIENT_CHAT_MESSAGE_H
