#pragma once

#include <sstream>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

namespace exerbike
{
    class exerbike_websocket_type
    {
    public:
        AsyncWebServer& webserver_;
        AsyncWebSocket ws_handler_;

        exerbike_websocket_type(AsyncWebServer& webserver)
            : webserver_(webserver)
            , ws_handler_("/exerbike")
        {
        }

        void setup()
        {
            ws_handler_.onEvent(
                [this](
                    AsyncWebSocket *websocket,
                    AsyncWebSocketClient *client,
                    AwsEventType type,
                    void *arg,
                    uint8_t *data,
                    size_t len) {
                    on_event(websocket, client, type, arg, data, len);                    
                });
            webserver_.addHandler(&ws_handler_);
        }

        void loop()
        {
            ws_handler_.cleanupClients();
        }
        
        void broadcast(float resistance, float target_resistance, float work_kcal)
        {
            std::stringstream ss;
            ss << "{\"resistance\": " << resistance
               << ",\"target_resistance\": " << target_resistance
               << ",\"work_kcal\": " << work_kcal << "}";
            auto s = ss.str();
            ws_handler_.textAll(s.data(), s.size());
        }
    
    private:
        void on_event(AsyncWebSocket *websocket, AsyncWebSocketClient *client, AwsEventType type,
                                void *arg, uint8_t *data, size_t len)
        {
            switch (type)
            {
            case WS_EVT_CONNECT:
            {
                Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
                // std::string s = std::to_string(ledState);
                // websocket_handler.text(client->id(), s.data(), s.size());
            }
            break;
            case WS_EVT_DISCONNECT:
                Serial.printf("WebSocket client #%u disconnected\n", client->id());
                break;
            case WS_EVT_DATA:
                // handleWebSocketMessage(arg, data, len);
                break;
            case WS_EVT_PONG:
            case WS_EVT_ERROR:
                break;
            }
        }


    };

}
