#include "ChatManager.h"

void ChatManager::begin() {
    clear();
}

void ChatManager::addMessage(const char* nickname, const char* text) {
    ChatMessage msg;
    msg.id = ++_totalMessages;

    // Copy nickname (sanitize or trim)
    strncpy(msg.nickname, nickname, sizeof(msg.nickname) - 1);
    msg.nickname[sizeof(msg.nickname) - 1] = '\0';

    // Trim nickname whitespace if any
    size_t nickLen = strlen(msg.nickname);
    while (nickLen > 0 && isspace(msg.nickname[nickLen - 1])) {
        msg.nickname[--nickLen] = '\0';
    }
    if (strlen(msg.nickname) == 0) {
        strcpy(msg.nickname, "Anonymous");
    }

    // Copy text
    strncpy(msg.text, text, sizeof(msg.text) - 1);
    msg.text[sizeof(msg.text) - 1] = '\0';

    msg.timestampSec = millis() / 1000;

    // Maintain circular buffer
    if (_messages.size() >= MAX_MESSAGES) {
        _messages.erase(_messages.begin());
    }
    _messages.push_back(msg);
}

void ChatManager::clear() {
    _messages.clear();
    _totalMessages = 0;
}
