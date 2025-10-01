### Smart Pointers in C++
Smart pointers are a crucial feature in C++ that help manage dynamic memory automatically, reducing the risk of memory leaks and dangling pointers. They are part of the C++ Standard Library and are defined in the `<memory>` header. The three primary types of smart pointers are `std::unique_ptr`, `std::shared_ptr`, and `std::weak_ptr`.

#### 1. `std::unique_ptr`
- Represents exclusive ownership of a dynamically allocated object.
- Cannot be copied, only moved.
- Move semantics allow transferring ownership from one `unique_ptr` to another. It is used when you want a single owner for a resource.
- Custom deleters can be specified to define how the resource should be released.
- Automatically deletes the object when the `unique_ptr` goes out of scope.

```cpp

#include <memory>
#include <iostream>
void uniquePtrExample() {
    std::unique_ptr<int> ptr1(new int(42)); // Create a unique_ptr
    std::cout << "Value: " << *ptr1 << std::endl; // Access the value

    // std::unique_ptr<int> ptr2 = ptr1; // Error: cannot copy
    std::unique_ptr<int> ptr2 = std::move(ptr1); // Transfer ownership
    if (!ptr1) {
        std::cout << "ptr1 is now null after move." << std::endl;
    }
}

int main() {
    uniquePtrExample();
    return 0;
}
```

#### 2. `std::shared_ptr`
- Represents shared ownership of a dynamically allocated object.
- Can be copied, and multiple `shared_ptr` instances can point to the same object.
- Uses reference counting to keep track of how many `shared_ptr` instances point to the same object.
- Includes a pointer to control block, which manages the reference count, custom deleters, and the actual object.
- Automatically deletes the object when the last `shared_ptr` pointing to it is destroyed or reset.

```cpp
#include <memory>
#include <iostream>

void sharedPtrExample() {
    std::shared_ptr<int> ptr1 = std::make_shared<int>(42); // Create a shared_ptr
    {
        std::shared_ptr<int> ptr2 = ptr1; // Copy ptr1 to ptr2
        std::cout << "Value: " << *ptr2 << std::endl; // Access the value
        std::cout << "Reference count: " << ptr1.use_count() << std::endl; // Reference count
    } // ptr2 goes out of scope here

    std::cout << "Reference count after ptr2 goes out of scope: " << ptr1.use_count() << std::endl;
}

int main() {
    sharedPtrExample();
    return 0;
}
```

#### 3. `std::weak_ptr`
- Represents a non-owning reference to an object managed by `std::shared_ptr`.
- Does not affect the reference count of the object.
- Used to break circular references that can lead to memory leaks.
- Can be converted to `std::shared_ptr` using the `lock()` method, which returns a `shared_ptr` if the object is still alive, or a null `shared_ptr` if the object has been deleted.
```cpp
#include <memory>
#include <iostream>
void weakPtrExample() {
    std::shared_ptr<int> sharedPtr = std::make_shared<int>(42); // Create a shared_ptr
    std::weak_ptr<int> weakPtr = sharedPtr; // Create a weak_ptr from shared_ptr

    if (auto lockedPtr = weakPtr.lock()) { // Try to lock the weak_ptr
        std::cout << "Value: " << *lockedPtr << std::endl; // Access the value
    } else {
        std::cout << "The object has been deleted." << std::endl;
    }

    sharedPtr.reset(); // Delete the managed object

    if (auto lockedPtr = weakPtr.lock()) { // Try to lock the weak_ptr again
        std::cout << "Value: " << *lockedPtr << std::endl; // Access the value
    } else {
        std::cout << "The object has been deleted." << std::endl;
    }
}

int main() {
    weakPtrExample();
    return 0;
}
```
#### 4. `std::make_shared`

- **`std::make_shared<T>(...)`** is a function that:
  - Allocates a new object of type `T`.
  - Returns a `std::shared_ptr<T>` that owns the object.
  - Combines object creation and smart pointer management in one step.

- Why use `std::make_shared`?
    - It is efficient: allocates the object and control block together.
    - It is safe: prevents memory leaks and exceptions during construction.
    - It is convenient: no need to use `new` manually.

##### Example

```cpp
OrderPointer order = std::make_shared<Order>(OrderType::GoodTillCancel, 123, Side::Buy, 100, 10);
// Creates a new Order object and returns a shared_ptr managing it
```

### Summary
- Use `std::unique_ptr` when you want single ownership of a resource.
- Use `std::shared_ptr` when you need shared ownership and reference counting.
- Use `std::weak_ptr` to hold a non-owning reference to an object managed by `std::shared_ptr`, especially to avoid circular references.
- Always prefer smart pointers over raw pointers for dynamic memory management in modern C++ to ensure safety and clarity in your code.
- Use `std::make_shared` to create and manage objects with `std::shared_ptr` in a single, safe, and efficient step.

