# c-irc-server
A standalone RFC 1459 compliant IRC Server written in C99 which implements all commands required to communicate, interface with channels, and check server + channel status. Created for educational purposes as an exploration into networking, memory management, and the C language.

## Features
This project supports numerous commands required for chatting, channel access, channel + user mode modification, operator access, as well as a modifiable Message of the Day (MOTD) file that may be set as needed.  

After testing, the server was refactored from a `poll()`-based implementation to an `epoll()`-based implementation after researching historic problems with scaling (The C10k Problem) that plagued IRC implementations in the 90's. Due to this, the server can concurrently support 10k users with minimal latency loss (as long as they are not all in the same channel, due to the nature of relaying messages through channels.) 

A full overview of the commands implemented by the IRC server itself can be viewed by observing the numeric replies in `messages.h`.


## Performance

As I neared the end of development, I became interested in the process of scaling and stress testing my new server. This led to me becoming more familiar with the I/O multiplexing process, as well as the shift from a thread-based architecture to asynchronous event
loops as the Internet continued to develop. After researching this, I noticed the stark different in performance between `poll()` and `epoll()`, and became interested in graphing and documenting this performance change on my Linux machine, as I was unable to find
similar educational demonstrations online. This graph, of the before and after performance from both implementations on this project can be seen below:

`TODO: Add graph`

## Usage
### Prerequisites
* **Linux:** This server relies on `epoll()`, which is a Linux-specific I/O event polling system call. Therefore it will only function on a Linux system.
* **C Compiler:** GCC or Clang is OK, as long as it supports C99.
* **Make:** For building the project.

### Build and Run

1. Clone the repository and navigate to the project directory:
   ```bash
   git clone https://github.com/YourUsername/c-irc-server.git
   cd c-irc-server
   ```

2. Compile the server using `make`:
   ```bash
   make
   ```

3. Run the executable, passing your desired server operator password as the single argument:
   ```bash
   ./circ [operator_pass]
   ```

> **Note:** The `motd.txt` file can be modified directly within the root project directory in order to set a new Message of the Day for the server.

## Reflection

Originally, the idea for writing an IRC server popped up in my head a while ago, but I never really considered it further than something that would be "cool to do one day." However, recently, my philosophy for my personal development in programming has changed. I
realized that along the way, I became too fixated on the end goal of "getting good," instead of the process of engaging in the things that I truly found interested and learning instead as a byproduct. This led me to creating this personal project that I, along
with many others, found to be an incredibly educational project that I feel that many beginner/intermediate developers should undertake at least once, which I will expand upon this further to anyone that may be interested.

For the undertaking of a project like this, all pre-requisite material can be acquired from [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/), which if you are familiar with network programming at all I imagine you will have heard of this resource
before, as it seems to be a foundational text for the subject. However, one such resource that I have not seen mentioned *as* much (or at all) is the University of [University of Chicago's chirc](http://chi.cs.uchicago.edu/chirc/) assignment guide. I **do not** recommend
utilizing the pre-existing skeleton provided for this assignment, but instead using it as more of a progression guide, leaving the implementation entirely up to you.

These things, combined with reading the RFC specification (RFC 2810 & RFC 1459) provides the entirety of the foundational knowledge required to tackle this project. If desired, you could also create a distributed network of these servers to communicate with
each other, but that is beyond the scope of this current implementation (and my current knowledge.) Overall, it is a treasure trove of a project that will teach you about TCP fragmentation, I/O buffering techniques, memory+state management, and the skill
and pleasure of sifting through man pages. Highly recommended.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
