# FixedThreadPool

固定大小线程池实现，提供任务提交、直接添加任务与优雅关闭的能力。

主要接口（见 `include/FixedThreadPool.hpp`）：

- `FixedThreadPool(int num_threads = std::thread::hardware_concurrency())`：构造并启动线程池
- `template<typename F, typename... Args> auto submit(F&& f, Args&&... args)`：提交可返回值任务，返回 `std::future<...>`
- `void add_task(const Task &task)` / `void add_task(Task&& task)`：直接添加无 future 任务
- `void stop()`：优雅关闭线程池，等待所有任务完成

示例：

```cpp
#include "FixedThreadPool/FixedThreadPool.hpp"
#include <iostream>

int main() {
    shanchuan::FixedThreadPool pool(4);
    auto fut = pool.submit([](int a, int b){ return a + b; }, 2, 3);
    std::cout << "sum=" << fut.get() << std::endl;
    pool.stop();
}
```

实现细节说明：线程池使用 `SyncQueue` 作为任务缓冲区，提交任务在队列已满或线程池停止时会直接在调用者线程执行以避免任务丢失。
