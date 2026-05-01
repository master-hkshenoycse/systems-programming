# Simple Linux Shell (C)

A minimal yet powerful Unix-like shell implemented in C using low-level system calls such as `fork`, `exec`, `waitpid`, `kill`, and signal handling. This project demonstrates core operating system concepts including process management, job control, and signal propagation.

---

## 🚀 Features

### 1. Basic Command Execution

* Runs standard Linux commands like:

  ```bash
  ls
  pwd
  echo hello
  cat file.txt
  ```
* Uses `fork()` + `execvp()` for execution.

---

### 2. Custom Shell Prompt

* Displays current working directory:

  ```bash
  /home/user $
  ```

---

### 3. Built-in Commands

* `cd <dir>` → change directory
* `exit` → terminate shell gracefully

---

### 4. Exit Status Reporting

* Prints exit code after each foreground command:

  ```bash
  EXITSTATUS: 0
  ```

---

### 5. Background Execution (`&`)

* Run commands in background:

  ```bash
  sleep 10 &
  ```
* Shell does not wait.
* Prints notification when finished:

  ```bash
  Shell: Background process finished
  ```

---

### 6. Proper Zombie Reaping

* Uses `waitpid(..., WNOHANG)` to clean up background processes.

---

### 7. Signal Handling (Ctrl+C)

* Custom `SIGINT` handler:

  * Does NOT kill shell
  * Kills only foreground process
* Background processes remain unaffected.

---

### 8. Process Group Management

* Background processes moved to separate groups using:

  ```c
  setpgid(0, 0);
  ```
* Ensures correct signal isolation.

---

### 9. Serial Execution (`&&`)

* Executes commands sequentially:

  ```bash
  sleep 2 && sleep 3 && echo done
  ```
* Each command runs after previous completes.

---

### 10. Parallel Execution (`&&&`)

* Executes commands concurrently:

  ```bash
  sleep 5 &&& sleep 5 &&& sleep 5
  ```
* Shell waits until all complete.

---

### 11. Clean Exit Handling

* On `exit`:

  * Kills all background processes
  * Reaps all children
  * Frees allocated memory

---

## 🧠 Key Concepts Demonstrated

* Process creation and control (`fork`, `exec`)
* Parent-child synchronization (`waitpid`)
* Non-blocking process handling (`WNOHANG`)
* Signal handling (`SIGINT`)
* Process groups (`setpgid`)
* Foreground vs background job control
* Command parsing and tokenization

---

## 🛠️ Compilation & Execution

### Compile:

```bash
gcc my_shell.c -o my_shell
```

### Run:

```bash
./my_shell
```

---

## 🧪 Example Usage

```bash
/home/user $ ls
/home/user $ sleep 5 &
/home/user $ sleep 3 && echo done
/home/user $ sleep 5 &&& sleep 5
/home/user $ cd /tmp
/tmp $ exit
```

---

## ⚠️ Assumptions & Limitations

* Max input length: 1024 characters
* Max tokens: 64
* No support for:

  * Pipes (`|`)
  * Redirection (`>`, `<`)
  * Quoted strings
  * Advanced job control (`fg`, `bg`, `jobs`)
* Commands must be space-separated
* `&&` and `&&&` must have spaces around them

---

## 📁 Project Structure

```
shell-code/
 ├── my_shell.c
 └── README.md
```

---

## 📌 Future Improvements

* Add pipe support (`|`)
* Add I/O redirection
* Implement job control commands (`jobs`, `fg`, `bg`)
* Support quoted arguments
* Better parsing (shell grammar)

---

## 🧾 License

This project is for educational purposes and learning operating system concepts.

---

## 🙌 Acknowledgment

Built as part of a systems programming / operating systems lab to understand how Unix shells work internally.
