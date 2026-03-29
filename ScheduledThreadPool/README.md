# ScheduledThreadPool

定时/调度线程池，用于延迟或周期性任务调度，通常与 `TimerQueue` 配合使用。

常见用法（示例占位，请参考 `ScheduledThreadPool/include/ScheduledThreadPool.hpp`）：

```cpp
#include "ScheduledThreadPool/ScheduledThreadPool.hpp"

int main() {
    // 创建定时线程池并安排任务
    ThreadPond::ScheduledThreadPool pool(2);
    pool.schedule([]{ /* 延迟执行 */ }, std::chrono::milliseconds(100));
    pool.shutdown();
}
```

我可以根据你希望的文档深度进一步补充：函数签名、参数说明、异常与线程安全语义等。
