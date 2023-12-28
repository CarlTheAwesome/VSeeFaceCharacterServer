#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <CImg.h>
#include <fstream>

using json = nlohmann::json;
typedef websocketpp::server<websocketpp::config::asio> server;

void processVSeeFaceData(const json& data) {
    std::string characterName = data["character"]["name"];
    std::string imageData = data["character"]["imageData"];
    
    std::cout << "Character Name: " << characterName << std::endl;

    // Decode base64 image data
    std::vector<unsigned char> decodedImage;
    cimg_library::CImg<unsigned char> image;

    cimg_library::CImg<unsigned char> rawData(1, imageData.length(), 1, 1, 0);
    cimg_forX(rawData, i) rawData(i) = static_cast<unsigned char>(imageData[i]);

    image = rawData;
    
    // Perform image processing with CImg (modify as needed)
    image.blur(2.5);

    // Save the processed image
    image.save("./processed_image.jpg");
}

void serveHTML(server& WebSocketServer, websocketpp::connection_hdl hdl) {
    std::ifstream file("./index.html", std::ios::binary);

    if (file.is_open()) {
        std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        websocketpp::http::response<websocketpp::http::string_body> res;
        res.set_body(htmlContent);
        res.set_status(200);

        WebSocketServer.send(hdl, res);
    } else {
        websocketpp::http::response<websocketpp::http::string_body> res;
        res.set_status(404);

        WebSocketServer.send(hdl, res);
    }
}

int main() {
    server WebSocketServer;

    WebSocketServer.init_asio();

    WebSocketServer.set_message_handler([&](websocketpp::connection_hdl hdl, server::message_ptr msg) {
        std::string payload = msg->get_payload();

        try {
            // Parse the JSON payload
            json data = json::parse(payload);

            // Process VSeeFace data
            processVSeeFaceData(data);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }

        // Respond to the client
        WebSocketServer.send(hdl, "Received your message", msg->get_opcode());
    });

    WebSocketServer.set_http_handler(&serveHTML);

    WebSocketServer.listen(9002);
    WebSocketServer.start_accept();

    WebSocketServer.clear_access_channels(websocketpp::log::alevel::all);

    WebSocketServer.run();
    return 0;
}
