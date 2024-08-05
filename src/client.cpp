#include <iostream>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

class Client {
public:
    Client(const std::string& server_ip, int port, const std::string& username);
    void start();

private:
    void receive_messages();
    void send_messages();

    std::string server_ip;
    int port;
    std::string username;
    int client_socket;
};

Client::Client(const std::string& server_ip, int port, const std::string& username)
    : server_ip(server_ip), port(port), username(username) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        throw std::runtime_error("Cannot create socket");
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &hint.sin_addr);

    if (connect(client_socket, (sockaddr*)&hint, sizeof(hint)) == -1) {
        throw std::runtime_error("Cannot connect to server");
    }

    // Send username to server
    send(client_socket, username.c_str(), username.size() + 1, 0);
}

void Client::start() {
    std::thread(&Client::receive_messages, this).detach();
    send_messages();
}

void Client::receive_messages() {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, 4096);
        int bytes_received = recv(client_socket, buffer, 4096, 0);
        if (bytes_received <= 0) {
            std::cout << "Server disconnected\n";
            close(client_socket);
            break;
        }
        std::string msg(buffer, 0, bytes_received);
        std::cout << msg << std::endl;
    }
}

void Client::send_messages() {
    std::string user_input;
    while (true) {
        std::getline(std::cin, user_input);
        if (user_input == "quit") break;
        std::string message = username + ": " + user_input;
        send(client_socket, message.c_str(), message.size() + 1, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./client <username>\n";
        return -1;
    }

    std::string username = argv[1];

    try {
        Client client("127.0.0.1", 54001, username); // Use the same port as the server
        client.start();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}
