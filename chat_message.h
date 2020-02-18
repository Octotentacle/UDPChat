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

std::string to_str(MessageType type) {
    switch (type) {
        case MessageType::kOnlineState: return "Client connection message";
        case MessageType::kSimpleMessage: return "Simple text message";
        case MessageType::kPrivateMessage: return "Private text message";
        default: return "Unknown message";
    }
}

class ChatMessage {
public:

    static constexpr size_t kHeaderSize = 256;
    static constexpr size_t kMessageSize = 1024;

    ChatMessage() : data_(), type_(MessageType::kUnknown) {
        data_[0] = static_cast<char>(MessageType::kUnknown);
    }

    explicit ChatMessage(std::time_t time) : data_(), type_(MessageType::kOnlineState) {
        WriteToMessage(time);
    }

    explicit ChatMessage(char data[kMessageSize]) : data_() , type_(MessageType::kUnknown) {
        data_[0] = static_cast<char>(MessageType::kUnknown);
        memcpy(data_.data(), data, sizeof(char) * kMessageSize);
        Decode();
    }

    explicit ChatMessage(const std::string& textMessage) : data_(), type_(MessageType::kSimpleMessage) {
        data_[0] = static_cast<char>(MessageType::kSimpleMessage);
        for (size_t i = 0; i < std::max(kMessageSize - kHeaderSize, textMessage.size()); ++i) {
            data_[i + kHeaderSize] = textMessage[i];
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
        std::copy(begin, end, data_.begin() + kHeaderSize);
    }

    void WriteToMessage(const std::string& data, bool isPrivate = false) {
        type_ = isPrivate ? MessageType::kPrivateMessage : MessageType::kSimpleMessage;
        data_[0] = static_cast<char>(type_);
        for (size_t i = 0; i < std::max(kMessageSize - kHeaderSize, data.size()); ++i) {
            data_[i + kHeaderSize] = data[i];
        }
    }

    template <typename T>
    T ReadFromMessage() const {
        T ret;
        char* begin = reinterpret_cast<char*>(std::addressof(ret));
        std::copy(data_.begin() + kHeaderSize, data_.begin() + kHeaderSize + sizeof(T), begin);
        return ret;
    }

    std::string ReadText() const {
        if (type_ != MessageType::kSimpleMessage && type_ != MessageType::kPrivateMessage) {
            throw;
        }
        std::string ret{};
        for (size_t i = kHeaderSize; i < kMessageSize && data_[i] != 0; ++i) {
            ret.push_back(data_[i]);
        }
        return ret;
    }


private:
    MessageType type_;
    boost::array<char, kMessageSize> data_;

};


#endif //CLIENT_CHAT_MESSAGE_H
