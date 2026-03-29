# WorkStealingPool

工作窃取线程池实现，适合任务粒度较小且负载不均的场景。工作窃取可提高线程利用率与吞吐量。

主要概念：每个 worker 维护本地队列（通常为双端队列），空闲 worker 可从其它 worker 偷取任务执行。

示例（占位，具体接口以 `WorkStealingPool/include/WorkStealingPool.hpp` 为准）：

```cpp
#include "WorkStealingPool/WorkStealingPool.hpp"

int main() {
    ThreadPond::WorkStealingPool pool(4);
    pool.submit([]{ /* work */ });
    pool.shutdown();
}
```

如果需要，我可以扫描 `WorkStealingPool/include/` 并把示例改为准确的函数名与参数。
