# ⚡ Parallel Word Frequency Analysis in C

A high-performance system for analyzing the **top 10 most frequent words** in a large dataset using different execution models:
- Single-threaded (Naive)
- Multiprocessing
- Multithreading

The project evaluates performance, scalability, and efficiency using real-world data.

---

## 🚀 Overview
This project processes a large text dataset (e.g., `enwik8`) and compares three approaches:

- 🐢 Naive (Sequential)
- ⚙️ Multiprocessing (Parallel using processes)
- 🧵 Multithreading (Parallel using threads)

Execution time is measured and analyzed using **Amdahl’s Law**.

---

## 🎯 Objective
- Extract the **top 10 most frequent words**
- Compare execution time across different approaches
- Analyze scalability and parallel performance

---

## 🧠 Approaches

### 🔹 Naive (Single-threaded)
- Sequential processing  
- No parallelism  
- Simple but slow  

👉 From results:
- ~622 seconds execution time  

---

### 🔹 Multiprocessing
- Uses multiple child processes (`fork()`)  
- Shared memory via `mmap()`  
- Synchronization with semaphores  

#### ⚙️ Key Features
- Parallel chunk processing  
- Shared result merging  
- Avoids race conditions  

---

### 🔹 Multithreading
- Uses POSIX threads (`pthread`)  
- Shared memory by default  
- Synchronization using mutex  

#### ⚙️ Key Features
- Faster communication than processes  
- Lower overhead  
- Efficient for shared data  

---

## 📊 Performance Comparison

| Approach | Time (approx) |
|---------|--------------|
| Naive | ~622 sec |
| Multiprocessing (8) | ~210 sec |
| Multithreading (8) | ~205 sec |

👉 From page 15:
- Increasing threads/processes reduces execution time  
- Optimal range: **4–6 cores**  

---

## 📈 Amdahl’s Law Analysis
- Serial portion ≈ **9.5%**  
- Parallel portion ≈ **90.5%**  

👉 Insight:
- Maximum speedup is limited by serial part  
- Parallelization is highly effective  

---

## 🔧 Tech Stack
- C (Unix)  
- POSIX Threads (`pthread`)  
- Process API (`fork`, `waitpid`)  
- Shared Memory (`mmap`)  
- Semaphores  

---

👨‍💻 Author
Mohammad Omar
