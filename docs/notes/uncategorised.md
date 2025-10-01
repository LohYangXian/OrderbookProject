
### Const Member Functions and Const Objects in C++

In C++, the keyword `const` is used to define constants and to indicate that certain variables or member functions should not modify the state of an object. Const means constant, adding to functions means it won't modify the object. Const member functions can be called on const objects. Const objects are objects that cannot be modified after they are created. Const objects are used in situations where you want to ensure that an object remains unchanged, such as when passing it to functions or storing it in data structures that require immutability.

### When Creating a New Custom Class
- Public: Interface functions that can be called by users of the class.
- Private: Internal data members and helper functions that should not be accessed directly by users of the class.
- Use `const` member functions for functions that do not modify the state of the object. This allows them to be called on const objects. Example, getter functions.
- `Using` headers for type aliases to improve code readability. Example, `using Price = uint32_t;` to represent price in cents.
- Prefer `std::shared_ptr` for managing dynamic memory and ownership semantics.
- Use `std::make_shared<Type>(args...)` to create shared pointers, which is more efficient and safer than using `new`.

### Returning by Value vs. Reference

- **Return by Value** (e.g., `OrderId GetOrderId() const { return orderId_; }`)
  - Makes a copy of the member variable.
  - Good for small, simple types (like `int`, `double`, enums).
  - No need for `const` before the return type.

- **Return by Reference** (e.g., `const TradeInfo& GetBidTradeInfo() const { return bidTrade_; }`)
  - Returns a reference to the member variable.
  - Use `const` before the return type to prevent modification of the returned object.
  - Good for large or complex types (like `std::string`, `std::vector`, custom structs) to avoid unnecessary copies.

#### When to Use Each

- **Use reference (`const Type&`)** for large objects to avoid copying.
- **Use value** for small, simple types—copying is cheap and safe.