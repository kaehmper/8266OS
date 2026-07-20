#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H

#include <Arduino.h>
#include <vector>

struct ChatMessage {
    uint32_t id;
    char nickname[17]; // Max 16 chars + null
    char text[65];     // Max 64 chars + null
    uint32_t timestampSec; // relative uptime in seconds
};

class ChatManager {
public:
    static ChatManager& getInstance() {
        static ChatManager instance;
        return instance;
    }

    void begin();
    void addMessage(const char* nickname, const char* text);
    const std::vector<ChatMessage>& getMessages() const { return _messages; }
    uint32_t getMessageCount() const { return _totalMessages; }
    void clear();

private:
    ChatManager() : _totalMessages(0) {}

    std::vector<ChatMessage> _messages;
    uint32_t _totalMessages;
    static const size_t MAX_MESSAGES = 20;
};

#endif // CHAT_MANAGER_H
