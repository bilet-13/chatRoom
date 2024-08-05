# Simple Chatroom

This is a simple terminal-based chatroom application implemented in C++. It allows multiple users to connect to a server and chat with each other in real-time. 

## Features

- Real-time messaging between multiple clients
- Each client shows messages from all users, including their own messages
- Notifications when users join or leave the chatroom

## Prerequisites

- Docker
- C++11 or higher
- A C++ compiler (e.g., `g++`)

## Getting Started

Follow these instructions to set up and run the chatroom application using Docker.

### Building Docker Images and Run

1. **Clone the repository:**

```bash
git clone <repository-url>
cd <repository-directory>
```
2. **Build the Docker image for the server and clients:**

```bash
docker build -f build/Dockerfile.server -t chatroom-server .
docker build -f build/Dockerfile.client -t chatroom-client .

```

3. **Start the server:** 
```bash
docker run --name chatroom-server -d chatroom-server
```

4. **Open a new terminal window for each client and run:**
```bash
docker run --rm -it chatroom-client ./client <username>

```