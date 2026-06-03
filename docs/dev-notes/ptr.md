# 智能指针

## unique_ptr 与 shared_ptr 的区别

追求更高性能时使用 `unique_ptr`。

`unique_ptr` 使用引用不会发生所有权转移，必须明确使用 `move` 函数才会发生所有权转移。

`unique_ptr` 传值时必须使用 `move` 函数。

`shared_ptr` 传引用与 `unique_ptr` 一样不会发生性能损耗，但 `shared_ptr` 可能会被外部释放。

```c++
// 注意：转换后不要再手动 delete rawPtr，否则 uPtr 释放时会引发二次释放崩溃。

// 裸指针转 unique_ptr
int* rawPtr = new int(10);
std::unique_ptr<int> uPtr(rawPtr);  // 直接构造（最常用）
std::shared_ptr<int> sPtr(rawPtr);  // 直接构造

std::unique_ptr<int> uPtr = std::make_unique<int>(30);
std::shared_ptr<int> sPtr2 = std::move(uPtr);

// 数组指针转 unique_ptr
int* rawArray = new int[10];
std::unique_ptr<int[]> uPtr(rawArray); // 正确：会调用 delete[]

// =========================================================================== //
// lambda 中使用 unique_ptr 的问题

// 1.使用引用捕获，所有权不转移
auto ptr1 = std::make_unique<int>(100);
auto task1 = [&ptr1]() {
    // 在这里使用 ptr1
};
// unique_ptr 引用传递，此时 ptr1 不为 nullptr

// 2.使用 std::move 捕获，所有权转移进 Lambda
auto ptr2 = std::make_unique<int>(100);
auto task2 = [p = std::move(ptr2)]() {
    // 在这里使用 p
};
// unique_ptr 传值传递，此时 ptr2 == nullptr

// 3.lambda 中需要再次转移时，需要 mutable 关键字，否则 p 在 Lambda 内默认是 const
auto ptr3 = std::make_unique<int>(300);
auto task3 = [p = std::move(ptr3)]() mutable {
    // 在这里使用 p
    someFunction(std::move(p)); // 必须加 mutable，否则 p 在 Lambda 内默认是 const
};
// unique_ptr 传值传递，此时 ptr3 == nullptr

// =========================================================================== //

// 调用
[](std::unique_ptr<T>  p){}(std::move(ptr)     );  // 传值传递，所有权转移
[](std::unique_ptr<T>& p){}(          ptr      );  // 传引用传递，所有权不转移
[](                T * p){}(          ptr.get());  // 传裸指针，不转移所有权

// =========================================================================== //

void resetPtr(std::unique_ptr<int>& up, std::shared_ptr<int>& sp) {
    up.reset(new int(1)); // 外部的 unique_ptr 真的变了
    sp.reset(new int(2)); // 外部的 shared_ptr 也真的变了，原对象的计数会减 1
}
```

## 线程安全

`shared_ptr` 只有引用计数是线程安全的，指针以及指向的对象都不是线程安全的。

`unique_ptr` 设计上就只能一个对象独占，不是线程安全的。

`vector` / `list` 等标准容器只读时是线程安全的，写入时不是，且会造成迭代器失效。

* 更多
  * C++ 20 引入了 `std::atomic<std::shared_ptr<T>>`，用来解决多线程间共享 `shared_ptr` 的安全问题。
  * 对于 `vector`，如果需要高性能并发，建议寻找无锁容器（如 `tbb::concurrent_vector`）或经典的 Mutex 方案。
