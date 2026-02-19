# ft_irc

## Accessing the client vector
1. The client collection lives in `managerchannel` as a reference: `_clients`.
2. It is initialized through the constructor:
   ```cpp
   managerchannel::managerchannel(std::vector<client> &clients)
	   : _clients(clients)
   {
   }
   ```
3. Any method inside `managerchannel` can read or iterate over `_clients` directly.
4. Example iteration pattern (from `handle_input` in `src/client.cpp`):
   ```cpp
   for (size_t i = 0; i < _clients.size(); i++)
   {
	   client &other_client = _clients[i];
	   // use other_client.fd, other_client.nickname, etc.
   }
   ```

## Broadcasting a message (step by step)
1. Parse the incoming line and extract the message payload.
2. Loop over `_clients` to find every connected client.
3. Skip the sender by comparing file descriptors.
4. Send the message using `send()` to each remaining client.

Example (current logic in `src/client.cpp`):
```cpp
if (input.compare(0, 7, "PRIVMSG") == 0)
{
	std::string message = input.substr(8); // skip "PRIVMSG "
	for (size_t i = 0; i < _clients.size(); i++)
	{
		client &other_client = _clients[i];
		if (other_client.fd != c.fd)
		{
			send(other_client.fd, message.c_str(), message.size(), 0);
		}
	}
}
```

### Notes
- `c` is the sender, so `other_client.fd != c.fd` avoids echoing back.
- You can prepend a prefix or command before sending if needed.

## Getting the sender client
When a socket is readable, its file descriptor identifies the sender.
Use the `fd` from `pollfd` (or your receive function) to find the matching client:

```cpp
int sender_fd = _pfds[index].fd;
for (size_t i = 0; i < _client.size(); i++)
{
	if (_client[i].fd == sender_fd)
	{
		client &sender = _client[i];
		// sender is the client who sent the message
		break;
	}
}
```

## Parsing incoming messages (step by step)
IRC messages usually end with `\r\n`. Use the client's buffer to assemble full lines:

1. Append newly received data to `client.buffer`.
2. While `\r\n` exists, extract a complete line.
3. Parse command + parameters from that line.

Example buffering pattern:
```cpp
c.buffer += received_chunk;

size_t pos;
while ((pos = c.buffer.find("\r\n")) != std::string::npos)
{
	std::string line = c.buffer.substr(0, pos);
	c.buffer.erase(0, pos + 2);

	// line now contains one full command
}
```

Simple command/params split:
```cpp
std::string command;
std::string params;
size_t space = line.find(' ');
if (space == std::string::npos)
	command = line;
else
{
	command = line.substr(0, space);
	params = line.substr(space + 1);
}
```

## `struct client` field meanings
- `buffer`: stores partial data until a full `\r\n` line is complete.
- `fd`: socket descriptor, main identifier for the client.
- `status`: your custom connection state (e.g., AUTH/READY).
- `pass_ok`, `user_ok`, `regestred`: authentication/registration flags.

## How `managerchannel *channel` fits
Initialize the channel with the `_client` vector so it can broadcast:

```cpp
channel = new managerchannel(_client);
```

Then route parsed commands to it:
```cpp
channel->handle_input(line, sender);
```

## Teammate requirements (spec summary)
Your teammate must implement the following features in their part:

- Authenticate and register clients (PASS, NICK, USER).
- Join channels, send/receive private messages.
- Forward every channel message to all members.
- Support operators and regular users.
- Implement operator-only commands:
  - `KICK` — eject a user from channel
  - `INVITE` — invite user to channel
  - `TOPIC` — view/change channel topic
  - `MODE` — change channel modes:
	 - `i`: invite-only
	 - `t`: topic restricted to operators
	 - `k`: channel key (password)
	 - `o`: give/take operator privilege
	 - `l`: user limit

## Step-by-step: access data from `_client`
1. The main vector is `server::_client`.
2. Each element is a `client` struct with fields:
	- `buffer`, `fd`, `status`, `pass_ok`, `user_ok`, `regestred`.
3. Read or update any client like this:
	```cpp
	client &c = _client[i];
	int fd = c.fd;
	bool registered = c.regestred;
	```
4. To loop all clients:
	```cpp
	for (size_t i = 0; i < _client.size(); i++)
	{
		 client &c = _client[i];
	}
	```

## Step-by-step: find the sender
1. Use `pollfd.fd` (or the socket you just read from).
2. Match it in `_client` by comparing `fd`:
	```cpp
	int sender_fd = _pfds[index].fd;
	for (size_t i = 0; i < _client.size(); i++)
	{
		 if (_client[i].fd == sender_fd)
		 {
			  client &sender = _client[i];
			  break;
		 }
	}
	```

## Step-by-step: parse messages
1. Append new data into `client.buffer`.
2. Extract lines separated by `\r\n`.
3. Split each line into `command` and `params`.