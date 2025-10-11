# Types of C++ Locks

C++ provides several types of locks to manage concurrent access to shared resources in multi-threaded applications. Here are some of the most commonly used locks in C++:

1. **std::mutex**: This is the most basic type of lock in C++. It provides mutual exclusion, allowing only one thread to access a shared resource at a time. You can use `std::lock_guard` or `std::unique_lock` to manage the locking and unlocking of a mutex.

   Example:
   ```cpp
   std::mutex mtx;
   void threadFunction() {
       std::lock_guard<std::mutex> lock(mtx);
       // Critical section
   }
   ```
scoped_lock automatically releases the lock when it goes out of scope.
std::scoped_lock is a C++17 feature that can lock multiple mutexes at once, preventing deadlocks when locking multiple resources.

   Example:
   ```cpp
   std::mutex mtx1, mtx2;
   void threadFunction() {
       std::scoped_lock lock(mtx1, mtx2);
       // Critical section
   }
   ```


2. **std::recursive_mutex**: This type of mutex allows the same thread to lock it multiple times without causing a deadlock. It is useful when a function that locks a mutex calls another function that also locks the same mutex.


    Example:
    ```cpp
    std::recursive_mutex rec_mtx;
    void recursiveFunction(int count) {
         if (count <= 0) return;
         std::lock_guard<std::recursive_mutex> lock(rec_mtx);
         // Critical section
         recursiveFunction(count - 1);
    }
    ```

3. **std::shared_mutex**: This mutex allows multiple threads to read a shared resource simultaneously (shared access) but only one thread to write to it (exclusive access). It is useful for scenarios where reads are more frequent than writes. You can use `std::shared_lock` for shared access and `std::unique_lock` for exclusive access.


    Example:
    ```cpp
    std::shared_mutex sh_mtx;
    void readFunction() {
         std::shared_lock<std::shared_mutex> lock(sh_mtx);
         // Read from shared resource
    }
    void writeFunction() {
         std::unique_lock<std::shared_mutex> lock(sh_mtx);
         // Write to shared resource
    }
    ```


4. **std::timed_mutex**: This mutex allows a thread to attempt to lock it for a specified duration. If the mutex cannot be locked within that time, the attempt fails. This is useful for avoiding deadlocks in certain scenarios.

    Example:
    ```cpp
    std::timed_mutex t_mtx;
    void threadFunction() {
         if (t_mtx.try_lock_for(std::chrono::milliseconds(100))) {
             // Critical section
             t_mtx.unlock();
         } else {
             // Failed to acquire lock
         }
    }
    ```


5. **std::shared_timed_mutex**: This is a combination of `std::shared_mutex` and `std::timed_mutex`. It allows multiple threads to read simultaneously and provides timed locking for exclusive access.


    Example:
    ```cpp
    std::shared_timed_mutex sh_t_mtx;
    void readFunction() {
         std::shared_lock<std::shared_timed_mutex> lock(sh_t_mtx);
         // Read from shared resource
    }
    void writeFunction() {
         if (sh_t_mtx.try_lock_for(std::chrono::milliseconds(100))) {
             // Write to shared resource
             sh_t_mtx.unlock();
         } else {
             // Failed to acquire lock
         }
    }
    ```


6. **std::atomic**: While not a lock in the traditional sense, `std::atomic` provides a way to perform atomic operations on variables, ensuring that operations on these variables are thread-safe without the need for explicit locks. This is useful for simple data types like integers and pointers.

    Example:
    ```cpp
    std::atomic<int> atomicCounter(0);
    void increment() {
         atomicCounter.fetch_add(1);
    }
    ```

7. **std::condition_variable**: This is not a lock itself but is often used in conjunction with mutexes to synchronize threads. It allows threads to wait for certain conditions to be met before proceeding.

    Example:
    ```cpp
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;

    void waitFunction() {
         std::unique_lock<std::mutex> lock(mtx);
         cv.wait(lock, [] { return ready; });
         // Proceed when ready is true
    }

    void signalFunction() {
         {
             std::lock_guard<std::mutex> lock(mtx);
             ready = true;
         }
         cv.notify_one();
    }
    ```

These locks and synchronization primitives are essential tools for managing concurrency in C++ applications. The choice of which to use depends on the specific requirements of your application, such as the frequency of reads vs. writes, the need for recursive locking, and the potential for deadlocks.