# IRC Server - Client Map Guide

## Client Struct Overview

```cpp
struct client
{
    std::string buffer;      // Received data buffer
    int         fd;          // File descriptor (socket)
    int         status;      // Status code, can define 0=connected, 1=authenticated, etc.
    bool        pass_ok;     // Password validated
    bool        user_ok;     // USER command received
    bool        nick_ok;     // NICK command received
    bool        regestred;   // Fully registered (passed all checks)

    // Client information
    std::string nickname;    // Client's nickname
    std::string username;    // Client's username
    std::string hostname;    // Client's hostname (optional, from getpeername)
    std::string realname;    // Client's real name (from USER command)
    std::string current_channel; // Channel client joined (empty if none)
    bool        is_operator;     // True if client is IRC operator
};
```

## Map Structure

```cpp
std::map<int, client> _clients;  // Key: file descriptor, Value: client struct
```

---

## 1. How to Access an Element in the Map

### Using `find()` (Recommended - Safe)
```cpp
int fd = 5; // example file descriptor
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    // Client found - access via it->second
    it->second.nickname = "john";
    it->second.buffer += "new data";
    std::cout << "Client FD: " << it->second.fd << std::endl;
}
else {
    // Client not found
    std::cout << "Client not found!" << std::endl;
}
```

### Using `[]` Operator (Creates if Doesn't Exist)
```cpp
// WARNING: This creates a new entry if fd doesn't exist
_clients[fd].nickname = "john";
_clients[fd].pass_ok = true;
```

### Real Example from Code
```cpp
// From recv_data() function
std::map<int, client>::iterator it = _clients.find(fd);
if (it == _clients.end())
    return;
it->second.buffer += std::string(buffer, n);
```

---

## 2. How to Remove a Client from the Map

### Remove by File Descriptor
```cpp
int client_fd = 5;
_clients.erase(client_fd);
```

### Remove Using Iterator
```cpp
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    _clients.erase(it);
}
```

### Real Example from Code
```cpp
// From disconnect_client() function
void server::disconnect_client(size_t &index)
{
    int fd = _pfds[index].fd;
    close(fd);                    // Close the socket
    _clients.erase(fd);           // Remove from map
    _pfds.erase(_pfds.begin() + index);
    std::cout << "Client disconnected FD: " << fd << std::endl;
}
```

---

## 3. How to Search for an Element Inside the Struct

### Search by Nickname
```cpp
std::map<int, client>::iterator it;
for (it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second.nickname == "john") {
        // Found the client with nickname "john"
        std::cout << "Found client with FD: " << it->first << std::endl;
        break;
    }
}
```

### Search by Channel
```cpp
// Find all clients in a specific channel
std::map<int, client>::iterator it;
for (it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second.current_channel == "#general") {
        std::cout << "Client " << it->second.nickname 
                  << " is in #general" << std::endl;
    }
}
```

### Check if Client is Registered
```cpp
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    if (it->second.regestred) {
        std::cout << "Client is fully registered" << std::endl;
    }
}
```

---

## 4. How to Send Message to a Client

### Send to Specific Client by FD
```cpp
int client_fd = 5;
std::string message = "Hello from server\r\n";
send(client_fd, message.c_str(), message.length(), 0);
```

### Send Using Map Iterator
```cpp
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    std::string msg = "Welcome " + it->second.nickname + "\r\n";
    send(it->second.fd, msg.c_str(), msg.length(), 0);
}
```

### Send Error Message
```cpp
void send_error(int fd, const std::string &error_msg) {
    std::string msg = "ERROR :" + error_msg + "\r\n";
    send(fd, msg.c_str(), msg.length(), 0);
}
```

---

## 5. How to Send Message to All Clients

### Broadcast to Everyone
```cpp
std::string broadcast_msg = "Server announcement: Maintenance in 5 minutes\r\n";
std::map<int, client>::iterator it;
for (it = _clients.begin(); it != _clients.end(); ++it) {
    send(it->second.fd, broadcast_msg.c_str(), broadcast_msg.length(), 0);
}
```

### Broadcast Only to Registered Clients
```cpp
std::string msg = "Welcome to IRC server!\r\n";
std::map<int, client>::iterator it;
for (it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second.regestred) {
        send(it->second.fd, msg.c_str(), msg.length(), 0);
    }
}
```

### Broadcast to Specific Channel
```cpp
void broadcast_to_channel(const std::string &channel_name, const std::string &msg) {
    std::map<int, client>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.current_channel == channel_name) {
            send(it->second.fd, msg.c_str(), msg.length(), 0);
        }
    }
}
```

---

## 6. How to Remove Client from Channel

### Remove Client from Their Current Channel
```cpp
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    it->second.current_channel.clear(); // or = ""
}
```

### Complete Example - Client Leaves Channel
```cpp
void leave_channel(int fd) {
    std::map<int, client>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        std::string old_channel = it->second.current_channel;
        it->second.current_channel.clear();
        
        // Notify others in the channel
        std::string msg = ":" + it->second.nickname + " has left " + old_channel + "\r\n";
        broadcast_to_channel(old_channel, msg);
    }
}
```

---

## 7. How to Add Client to Channel

### Simple Add
```cpp
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    it->second.current_channel = "#general";
}
```

### Complete Example - Join Channel
```cpp
void join_channel(int fd, const std::string &channel_name) {
    std::map<int, client>::iterator it = _clients.find(fd);
    if (it != _clients.end() && it->second.regestred) {
        // Leave old channel if exists
        if (!it->second.current_channel.empty()) {
            leave_channel(fd);
        }
        
        // Join new channel
        it->second.current_channel = channel_name;
        
        // Send confirmation
        std::string msg = ":" + it->second.nickname + " JOIN " + channel_name + "\r\n";
        send(it->second.fd, msg.c_str(), msg.length(), 0);
        
        // Notify others
        broadcast_to_channel(channel_name, msg);
    }
}
```

---

## 8. What is a File Descriptor?

A **file descriptor (fd)** is a **small non-negative integer** that acts as a unique identifier for an open file, socket, or I/O resource in your program.

### Key Points:
- **Reference Number**: Think of it like a ticket number - the OS gives you a number to reference your socket
- **Unique per Process**: Each open socket/file gets its own fd
- **Standard FDs**: 
  - `0` = stdin (standard input)
  - `1` = stdout (standard output)
  - `2` = stderr (standard error)
  - `3+` = your sockets, files, etc.

### Example:
```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);  // Returns fd = 3
int client_fd = accept(server_fd, NULL, NULL);     // Returns fd = 4

// Now you use these numbers:
send(client_fd, "Hello", 5, 0);  // Send to fd 4
recv(client_fd, buffer, 1024, 0); // Receive from fd 4
close(client_fd);                 // Close fd 4
```

---

## 9. What is a Socket?

A **socket** is an **endpoint for network communication** - like a telephone connection between two programs.

### Types of Sockets:
1. **Stream Socket (SOCK_STREAM)**: TCP - reliable, ordered, connection-based
2. **Datagram Socket (SOCK_DGRAM)**: UDP - unreliable, connectionless

### Socket Lifecycle:

#### Server Side:
```cpp
1. socket()   → Create socket
2. bind()     → Bind to address/port
3. listen()   → Mark as passive (waiting for connections)
4. accept()   → Accept incoming connection (returns new socket for client)
5. send/recv  → Communicate with client
6. close()    → Close connection
```

#### Client Side:
```cpp
1. socket()   → Create socket
2. connect()  → Connect to server
3. send/recv  → Communicate
4. close()    → Close connection
```

### Real Example from Your Code:
```cpp
// Server creates socket
this->_fd_server = socket(AF_INET, SOCK_STREAM, 0);

// Bind to address
bind(this->_fd_server, (struct sockaddr*)&_addr, sizeof(_addr));

// Listen for connections
listen(this->_fd_server, 10);

// Accept client
client_fd = accept(_fd_server, NULL, NULL);
```

---

## 10. What is a Server?

A **server** is a program that:
- **Listens** for incoming client connections
- **Accepts** multiple clients simultaneously
- **Manages** communication and data flow
- **Processes** requests and sends responses

### IRC Server Responsibilities:
1. **Authentication**: Verify passwords, nicknames
2. **Channel Management**: Create, join, leave channels
3. **Message Routing**: Forward messages between clients
4. **User Management**: Track connected users
5. **Protocol Enforcement**: Ensure IRC protocol compliance

### Server Architecture (Your Code):
```
┌─────────────────────────────────────┐
│         Server Socket (_fd_server)  │  ← Listens on port
│         Bound to INADDR_ANY:port    │
└────────────┬────────────────────────┘
             │
             │ accept() creates new sockets
             │
     ┌───────┴───────┬───────────┬─────────┐
     │               │           │         │
┌────▼────┐    ┌─────▼────┐ ┌───▼─────┐  ...
│ Client1 │    │ Client2  │ │ Client3 │
│  (fd=4) │    │  (fd=5)  │ │  (fd=6) │
└─────────┘    └──────────┘ └─────────┘
```

### Event Loop (poll):
```cpp
while (true) {
    poll(_pfds.data(), _pfds.size(), -1);  // Wait for events
    
    for each socket with data:
        if (socket == server_socket)
            accept_connection();  // New client
        else
            recv_data();          // Data from existing client
}
```

---

## 11. Why Do Messages End with `\r\n`?

### IRC Protocol Requirement
**`\r\n`** (Carriage Return + Line Feed) is the **official IRC protocol standard** defined in RFC 1459.

### Breaking Down `\r\n`:
- **`\r`** = Carriage Return (ASCII 13) - moves cursor to start of line
- **`\n`** = Line Feed (ASCII 10) - moves cursor down one line
- **`\r\n`** = Standard line ending from old teletype machines

### Why Both?
1. **Historical**: Comes from teletype machines that needed both commands
2. **Protocol Standard**: IRC specification mandates `\r\n`
3. **Message Delimiter**: Separates individual commands in the stream

### Example:
```cpp
// Client sends this raw data:
"NICK john\r\nUSER john 0 * :John Doe\r\nJOIN #general\r\n"

// Server receives and splits by \r\n:
Command 1: "NICK john"
Command 2: "USER john 0 * :John Doe"
Command 3: "JOIN #general"
```

### Extraction Logic (Your Code):
```cpp
std::string server::Extract_data(client &c)
{
    std::string &buf = c.buffer;
    
    size_t pos = buf.find('\n');
    if (pos == std::string::npos)
        return "";  // Incomplete message, wait for more data
    
    std::string line = buf.substr(0, pos);
    buf.erase(0, pos + 1);
    
    // Remove trailing '\r' if present
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1, 1);
    
    return line;  // Clean command without \r\n
}
```

---

## 12. How std::map Works

### Definition
```cpp
std::map<Key, Value>
std::map<int, client> _clients;  // Key=int (fd), Value=client struct
```

### Internal Structure
- **Binary Search Tree** (typically Red-Black Tree)
- **Sorted by Key**: Keys are always in ascending order
- **Unique Keys**: Each key appears only once

### Complexity
| Operation | Time Complexity |
|-----------|----------------|
| `find()`  | O(log n)       |
| `insert()`| O(log n)       |
| `erase()` | O(log n)       |
| `[]`      | O(log n)       |

### Visualization
```
_clients map:
         ┌────────────┐
         │  [5, bob]  │
         └──┬──────┬──┘
            │      │
    ┌───────┘      └───────┐
    │                      │
┌───▼────────┐      ┌──────▼──────┐
│ [3, alice] │      │ [7, charlie]│
└────────────┘      └─────────────┘
```

### Iterator Usage
```cpp
std::map<int, client>::iterator it;

// it->first  = key (int fd)
// it->second = value (client struct)

for (it = _clients.begin(); it != _clients.end(); ++it) {
    std::cout << "FD: " << it->first << std::endl;
    std::cout << "Nickname: " << it->second.nickname << std::endl;
}
```

### Common Operations
```cpp
// Insert
_clients[4] = new_client;
_clients.insert(std::make_pair(4, new_client));

// Find
std::map<int, client>::iterator it = _clients.find(4);

// Check if exists
if (_clients.find(4) != _clients.end()) {
    // Key 4 exists
}

// Erase
_clients.erase(4);

// Count (returns 0 or 1)
if (_clients.count(4) > 0) {
    // Key exists
}

// Size
size_t num_clients = _clients.size();

// Clear all
_clients.clear();
```

---

## Code Flow Summary

### 1. New Client Connects
```cpp
accept_connection()
    ↓
Create new client struct
    ↓
_clients[client_fd] = __client;  // Add to map
    ↓
Add to poll array (_pfds)
```

### 2. Client Sends Data
```cpp
recv_data()
    ↓
Find client: it = _clients.find(fd)
    ↓
Append to buffer: it->second.buffer += data
    ↓
Extract complete lines (split by \r\n)
    ↓
Process each command: handle_input(line, client)
```

### 3. Client Disconnects
```cpp
disconnect_client()
    ↓
close(fd)           // Close socket
    ↓
_clients.erase(fd)  // Remove from map
    ↓
Remove from poll array
```

---

## Best Practices

### ✅ Always Check if Key Exists
```cpp
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) {
    // Safe to access it->second
}
```

### ✅ Use Iterators for Modifications During Loop
```cpp
std::map<int, client>::iterator it = _clients.begin();
while (it != _clients.end()) {
    if (should_remove(it->second)) {
        it = _clients.erase(it);  // erase returns next iterator
    } else {
        ++it;
    }
}
```

### ❌ Don't Modify Map Size While Iterating (Wrong Way)
```cpp
// WRONG - undefined behavior
for (it = _clients.begin(); it != _clients.end(); ++it) {
    _clients.erase(it->first);  // Don't do this!
}
```

### ✅ Buffer Handling
```cpp
// Accumulate partial messages
it->second.buffer += new_data;

// Extract complete messages only
while (complete_message_exists) {
    process(extract_complete_message());
}
```

---

## Additional Notes

- **File Descriptor** = Unique ID for each socket
- **Socket** = Communication endpoint
- **Map** = Fast lookup data structure (Key → Value)
- **Buffer** = Temporary storage for incomplete messages
- **`\r\n`** = IRC message delimiter (protocol requirement)
- Always validate before accessing map elements
- Use `find()` to avoid creating unwanted entries

---

## Quick Reference

```cpp
// Access client
std::map<int, client>::iterator it = _clients.find(fd);
if (it != _clients.end()) { /* use it->second */ }

// Remove client
_clients.erase(fd);

// Search by property
for (it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second.nickname == "john") { /* found */ }
}

// Send to one client
send(it->second.fd, msg.c_str(), msg.length(), 0);

// Broadcast to all
for (it = _clients.begin(); it != _clients.end(); ++it) {
    send(it->second.fd, msg.c_str(), msg.length(), 0);
}

// Add to channel
it->second.current_channel = "#general";

// Remove from channel
it->second.current_channel.clear();
```
