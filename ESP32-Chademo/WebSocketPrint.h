#ifndef WebSocketPrint_h
#define WebSocketPrint_h

#include <ESPAsyncWebServer.h>

class WebSocketPrint
{
    public:
      WebSocketPrint(AsyncWebSocket& socket);
      void message(String message);

    private:
      AsyncWebSocket& webSocket;
};

#endif
