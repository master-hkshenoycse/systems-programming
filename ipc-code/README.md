# IPC Code Collection

This directory contains implementations of various **Inter-Process Communication (IPC)** mechanisms in Linux using C. These programs demonstrate how processes communicate, synchronize, and exchange data using operating system primitives.

The examples are intended for learning systems programming, operating systems concepts, and Unix/Linux process communication.

---

## Topics Covered

### 1. POSIX Shared Memory

Demonstrates communication between processes using shared memory segments.

#### Concepts

* `shm_open()`
* `ftruncate()`
* `mmap()`
* Shared address spaces
* Producer-consumer communication

#### Examples

* Basic shared memory producer and consumer
* Shared memory bounded buffer
* Slot reuse and synchronization

---

### 2. Unix Domain Sockets

Demonstrates local inter-process communication using Unix domain sockets.

#### Concepts

* `socket()`
* `bind()`
* `listen()`
* `accept()`
* `connect()`
* `read()` / `write()`
* Local client-server architecture

#### Examples

* Datagram socket communication
* Stream socket communication
* File transfer using Unix domain sockets

---

### 3. Pipes

Demonstrates communication using anonymous pipes.

#### Concepts

* `pipe()`
* `fork()`
* `dup2()`
* File descriptor redirection
* Parent-child communication

#### Examples

* Simple pipe communication
* Shell command pipelines

Example:

```bash
cat file.txt | grep hello
```

---

### 4. Signals

Demonstrates asynchronous communication between processes using signals.

#### Concepts

* `signal()`
* `kill()`
* Signal handlers
* Process synchronization
* User-defined signals (`SIGUSR1`, `SIGUSR2`)

#### Examples

* Bitstring transmission using signals
* Parent-child synchronization

---

### 5. Process Creation and Synchronization

Demonstrates process lifecycle management.

#### Concepts

* `fork()`
* `wait()`
* `waitpid()`
* Zombie processes
* Parent-child relationships

---

## Building

Most programs can be compiled using GCC.

### Shared Memory Programs

```bash
gcc producer.c -o producer -lrt
gcc consumer.c -o consumer -lrt
```

### Socket Programs

```bash
gcc socket-server.c -o socket-server
gcc socket-client.c -o socket-client
```

### Signal Programs

```bash
gcc bitstring-send.c -o bitstring-send
```

### Pipe Programs

```bash
gcc pipe-example.c -o pipe-example
```

---

## Running Examples

### Shared Memory

Terminal 1:

```bash
./consumer
```

Terminal 2:

```bash
./producer
```

---

### Unix Domain Socket File Transfer

Terminal 1:

```bash
./socket-server
```

Terminal 2:

```bash
./socket-client test.txt
```

---

### Signal Communication

```bash
./bitstring-send
```

Example:

```text
Please input a 8-bit bitstring: 10111100
[Parent] Input bitstring is     10111100
[Child] Received bitstring is   10111100
```

---

## Learning Objectives

These examples help understand:

* Inter-process communication (IPC)
* Process synchronization
* Shared memory programming
* Client-server communication
* Signal handling
* File descriptor management
* Pipe-based data transfer
* Linux system calls
* Operating system internals

---

## References

* Operating System Concepts – Silberschatz, Galvin, Gagne
* Linux Programmer's Manual
* POSIX System Programming Documentation

---

## Repository Purpose

This repository serves as a collection of hands-on systems programming exercises and IPC implementations for learning Linux internals, operating systems, and process communication mechanisms.
