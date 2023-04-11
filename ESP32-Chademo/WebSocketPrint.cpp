#include "WebSocketPrint.h"

WebSocketPrint::WebSocketPrint(AsyncWebSocket& socket) : webSocket { socket } {

}

void WebSocketPrint::message(String message) {
  webSocket.textAll("{\"info\": \"" + message + "\"}");
}
