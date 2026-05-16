*This project has been created as part of the 42 curriculum by haitaabe and eazmir and abhmidat.*

# IRC Server (`ircserv`)

## Description

This project is a custom Internet Relay Chat (IRC) server implemented in C++.
Its goal is to reproduce core IRC behavior by handling multiple client connections,
user registration/authentication, channel management, and message exchange using a
single server process.

In short, this repository provides a minimal IRC-like backend where clients can:
- connect and register to the server,
- create and join channels,
- send private and channel messages,
- manage channel modes and operator actions.

## Instructions

### Requirements

- Linux or Unix-like environment
- `c++` compiler with C++98 support
- `make`

### Compilation

From the repository root:

```bash
make
```

This generates the executable `ircserv`.

### Run

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypass
```

### Quick test with netcat

In another terminal:

```bash
nc -C 127.0.0.1 6667
```

Then send IRC commands manually, for example:

```text
PASS mypass
NICK user1
USER user1 0 * :User One
JOIN #general
PRIVMSG #general :hello everyone
```

## Features

- Client connection handling and event loop server behavior
- User registration flow (`PASS`, `NICK`, `USER`)
- Channel lifecycle (`JOIN`, `PART`) and channel messaging
- Operator tools and channel modes (e.g. invite-only, key, topic restrictions, limit)

## Project Structure

```text
.
├── include/
│   ├── authentication.hpp
│   ├── client.hpp
│   ├── server.hpp
│   ├── utls.hpp
│   └── channels/
│       └── channel.hpp
├── main/
│   └── main.cpp
├── src/
│   ├── authentication/
│   │   └── authentication.cpp
│   ├── bonus/
│   │   └── bot.cpp
│   ├── channels/
│   │   ├── channel_join_part.cpp
│   │   ├── channel_manager.cpp
│   │   ├── channel_message.cpp
│   │   ├── channel_modes.cpp
│   │   └── channel_operators.cpp
│   ├── server/
│   │   ├── accept.cpp
│   │   ├── bye.cpp
│   │   ├── clean.cpp
│   │   ├── desconnect.cpp
│   │   ├── extract.cpp
│   │   ├── HandleEvent.cpp
│   │   ├── HandleSignals.cpp
│   │   ├── receve.cpp
│   │   └── server.cpp
│   └── utils/
│       └── utls.cpp
└── Makefile
```

## Resources

Classic references used for this project:
- RFC 1459 (Internet Relay Chat Protocol): https://datatracker.ietf.org/doc/html/rfc1459
- RFC 2812 (IRC Client Protocol): https://datatracker.ietf.org/doc/html/rfc2812
- Beej’s Guide to Network Programming: https://beej.us/guide/bgnet/
- C++ reference (STL containers, strings, streams): https://en.cppreference.com/

### AI Usage

AI was used as a support tool for:
- improving wording and structure of documentation,
- reviewing code style consistency,
- suggesting refactoring options for readability (without changing intended behavior),
- generating and validating command examples for build/run/testing instructions.

AI was **not** used to replace project understanding, protocol study, or final decision-making.