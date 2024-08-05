#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unordered_map>

class Server {
public:
    Server(int port);
    void start();

private:
    void accept_clients();
    void handle_client(int client_socket);
    void broadcast_message(const std::string& message);

    int port;
    int server_socket;
    std::unordered_map<int, std::string> clients; // Map client socket to username
    std::mutex clients_mutex;
};

Server::Server(int port) : port(port) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        throw std::runtime_error("Cannot create socket");
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&hint, sizeof(hint)) == -1) {
        close(server_socket);
        throw std::runtime_error("Cannot bind to IP/Port");
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        close(server_socket);
        throw std::runtime_error("Cannot listen");
    }
}

void Server::start() {
    std::thread(&Server::accept_clients, this).detach();
    std::cout << "Server started on port " << port << std::endl;
    while (true);
}

void Server::accept_clients() {
    while (true) {
        sockaddr_in client;
        socklen_t client_size = sizeof(client);
        int client_socket = accept(server_socket, (sockaddr*)&client, &client_size);
        if (client_socket == -1) {
            std::cerr << "Problem with client connecting\n";
            continue;
        }

        // Receive username from client
        char username_buffer[4096];
        memset(username_buffer, 0, 4096);
        recv(client_socket, username_buffer, 4096, 0);
        std::string username(username_buffer);

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients[client_socket] = username;
        }

        // Announce the new client
        std::string join_message = username + " joined the chatroom";
        broadcast_message(join_message);

        std::thread(&Server::handle_client, this, client_socket).detach();
    }
}

void Server::handle_client(int client_socket) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, 4096);
        int bytes_received = recv(client_socket, buffer, 4096, 0);
        if (bytes_received <= 0) {
            close(client_socket);
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                std::string username = clients[client_socket];
                clients.erase(client_socket);

                // Announce the client has left
                std::string leave_message = username + " left the chatroom";
                broadcast_message(leave_message);
            }
            break;
        }

        std::string msg(buffer, 0, bytes_received);
        broadcast_message(msg);
    }
}

void Server::broadcast_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        send(client.first, message.c_str(), message.size() + 1, 0);
    }
}

int main() {
    try {
        Server server(54001); // Use port 54001 or another port that is available
        server.start();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}
