# Producer–Consumer problem

> A multithreaded implementation of the classic Producer–Consumer synchronization problem using Rust, a bounded circular buffer, Mutex, Condvar, and shared thread state.

---

## The Core Idea

Two threads.

One shared buffer.

One rule:

**The Producer must never produce when the buffer is full.  
The Consumer must never consume when the buffer is empty.**

The entire implementation revolves around making these two threads safely cooperate while accessing the same bounded buffer.

Instead of continuously checking the buffer, a thread waits when it cannot proceed and wakes up when the required condition changes.

---

## Architecture

```text
                         SHARED STATE
                              │
                    ┌─────────┴─────────┐
                    │                   │
                  Mutex              Condvar
                    │                   │
                    ▼                   │
             Circular Buffer            │
                    ▲                   │
                    │                   │
             ┌──────┴──────┐            │
             │             │            │
             ▼             ▼            │
         Producer       Consumer        │
          Thread         Thread         │
             │             │            │
             └──────┬──────┘            │
                    │                   │
                    └──── Synchronize ──┘

```
---
## Producer Flow

```
                        Generate Item
                             │
                             ▼
                        Acquire Mutex
                             │
                             ▼
                       Is Buffer Full?
                          /       \
                        YES        NO
                         │          │
                         ▼          ▼
                       WAIT       Produce
                         │          │
                         │          ▼
                         │       Notify
                         │          │
                         └──────────┤
                                    ▼
                              Release Mutex
                                    │
                                    ▼
                                 Continue

```
---
## Consumer Flow

```
                        Acquire Mutex
                             │
                             ▼
                      Is Buffer Empty?
                          /       \
                        YES        NO
                         │          │
                         ▼          ▼
                       WAIT       Consume
                         │          │
                         │          ▼
                         │       Notify
                         │          │
                         └──────────┤
                                    ▼
                              Release Mutex
                                    │
                                    ▼
                                 Continue
```
---
## Current Configuration 

    Buffer Capacity : 5
    Items           : 20
    Producer        : 1 thread
    Consumer        : 1 thread
    Buffer Type     : Circular Buffer
    Synchronization : Mutex + Condvar
    Shared State    : Arc
    Dependencies    : None
---
## Technology

    Language         → Rust
    Threading        → std::thread
    Shared Ownership → Arc
    Mutual Exclusion → Mutex
    Synchronization  → Condvar
    Data Structure   → Bounded Circular Buffer
    Dependencies     → None
---
## Running it

  ### Clone the repository
  ```
    git clone <repository-url>
    cd producer-consumer
  ```
    
  ### Check
    
  ```
    cargo check
  ```
    
  ### Run
  ```
    cargo run
  ```
---
## Project Structure

```
producer-consumer/
│
├── Cargo.toml
├── Cargo.lock
│
├── src/
│   └── main.rs
│
└── README.md
```
---
## Author
     H Nidhish Sheka
     NNM24IS289
