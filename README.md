# ft_irc

Minimal IRC server implementation for the 42 network project.

## Subject
Implement a small, RFC-like IRC server in C++98 following 42 school's rules: respect the norme, use only allowed functions, provide required commands and responses for basic functionality.

## Features
- PASS / NICK / USER registration flow
- JOIN / PART / TOPIC / MODE / INVITE / KICK
- PRIVMSG (channel and direct) and QUIT
- Channel operators, invite-only and password-protected channels
- Basic server-side `PRINT` command for debugging

## Project layout
- include/ — headers
- src/ — sources
- main/ — entry point
- Makefile

## Build
Build with the provided Makefile (flags set to 42 rules):

```sh
make
```

Compiler flags:
- -Wall -Wextra -Werror -std=c++98

To compile manually:
```sh
c++ -Wall -Wextra -Werror -std=c++98 src/*.cpp main/*.cpp -Iinclude -o ircserv
```

## Run
Start server:
```sh
./ircserv <port> <password>
```
Example:
```sh
./ircserv 6667 secret
```

Connect using netcat / telnet or an IRC client:
```sh
nc localhost 6667
```

Register a client:
- PASS <password>
- NICK <nick>
- USER <username> 0 * :Real Name

Example:
```
PASS secret
NICK alice
USER alice 0 * :Alice
```

## Testing
- Use multiple nc sessions to simulate users.
- Use `JOIN #channel`, `PRIVMSG #channel :hello`, `PART #channel`, and `QUIT`.
- Use `PRINT` (server console) to display connected clients for debugging.

## Notes for 42 evaluation
- Code targets C++98 and compiles with the flags listed above.
- Avoid banned functions and keep functions short to respect the norme.
- Ensure proper numeric replies where required by the subject.
- The implementation is a subset of IRC RFCs tailored to the project requirements.

## Author
haitaabe — eazmir