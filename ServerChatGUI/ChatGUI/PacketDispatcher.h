#pragma once

#include "Struct.h"

void GlobalPacketHandler(const char* data, int size);
void SetCurrentRegisterScreen(class RegisterScreen* screen);
void SetCurrentChatScreen(class ChatScreen* screen);

