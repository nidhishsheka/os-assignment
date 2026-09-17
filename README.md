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
## Sample Output
  ```
    Producer: I produced this guy -> 1  ,   Buffer count: 1
    Consumer: I just consumed 1 | Buffer cnt: 0 , So, that's how much is left.
    Producer: I produced this guy -> 2  ,   Buffer count: 1
    Consumer: I just consumed 2 | Buffer cnt: 0 , So, that's how much is left.
    Producer: I produced this guy -> 3  ,   Buffer count: 1
    Consumer: I just consumed 3 | Buffer cnt: 0 , So, that's how much is left.
    Producer: I produced this guy -> 4  ,   Buffer count: 1
    Producer: I produced this guy -> 5  ,   Buffer count: 2
    Consumer: I just consumed 4 | Buffer cnt: 1 , So, that's how much is left.
    Producer: I produced this guy -> 6  ,   Buffer count: 2
    Consumer: I just consumed 5 | Buffer cnt: 1 , So, that's how much is left.
    Producer: I produced this guy -> 7  ,   Buffer count: 2
    Producer: I produced this guy -> 8  ,   Buffer count: 3
    Consumer: I just consumed 6 | Buffer cnt: 2 , So, that's how much is left.
    Producer: I produced this guy -> 9  ,   Buffer count: 3
    Consumer: I just consumed 7 | Buffer cnt: 2 , So, that's how much is left.
    Producer: I produced this guy -> 10  ,   Buffer count: 3
    Producer: I produced this guy -> 11  ,   Buffer count: 4
    Consumer: I just consumed 8 | Buffer cnt: 3 , So, that's how much is left.
    Producer: I produced this guy -> 12  ,   Buffer count: 4
    Consumer: I just consumed 9 | Buffer cnt: 3 , So, that's how much is left.
    Producer: I produced this guy -> 13  ,   Buffer count: 4
    Producer: I produced this guy -> 14  ,   Buffer count: 5
    Consumer: I just consumed 10 | Buffer cnt: 4 , So, that's how much is left.
    Producer: I produced this guy -> 15  ,   Buffer count: 5
    Consumer: I just consumed 11 | Buffer cnt: 4 , So, that's how much is left.
    Producer: I produced this guy -> 16  ,   Buffer count: 5
    Producer: BUFFER is FULL... So, i'm waiting brother
    Consumer: I just consumed 12 | Buffer cnt: 4 , So, that's how much is left.
    Producer: I just woke up , Let me just check the buffer again
    Producer: I produced this guy -> 17  ,   Buffer count: 5
    Producer: BUFFER is FULL... So, i'm waiting brother
    Consumer: I just consumed 13 | Buffer cnt: 4 , So, that's how much is left.
    Producer: I just woke up , Let me just check the buffer again
    Producer: I produced this guy -> 18  ,   Buffer count: 5
    Producer: BUFFER is FULL... So, i'm waiting brother
    Consumer: I just consumed 14 | Buffer cnt: 4 , So, that's how much is left.
    Producer: I just woke up , Let me just check the buffer again
    Producer: I produced this guy -> 19  ,   Buffer count: 5
    Producer: BUFFER is FULL... So, i'm waiting brother
    Consumer: I just consumed 15 | Buffer cnt: 4 , So, that's how much is left.
    Producer: I just woke up , Let me just check the buffer again
    Producer: I produced this guy -> 20  ,   Buffer count: 5
    Producer: I'm Done.
    Consumer: I just consumed 16 | Buffer cnt: 4 , So, that's how much is left.
    Consumer: I just consumed 17 | Buffer cnt: 3 , So, that's how much is left.
    Consumer: I just consumed 18 | Buffer cnt: 2 , So, that's how much is left.
    Consumer: I just consumed 19 | Buffer cnt: 1 , So, that's how much is left.
    Consumer: I just consumed 20 | Buffer cnt: 0 , So, that's how much is left.
    Consumer: I'm done , what a great meal
    
    Main: Finally, Producer and Consumer have finished.
  ```
## Author
     H Nidhish Sheka
     NNM24IS289
