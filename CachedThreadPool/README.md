# CachedThreadPool

缓存/弹性线程池实现，用于短时高并发任务场景。线程数量可动态扩展以适应负载，空闲线程在一段时间后回收。

主要用法（示例占位，具体接口请查看 `CachedThreadPool/include/CachedThreadPool.hpp`）：

```cpp
#include "CachedThreadPool/CachedThreadPool.hpp"

int main() {
    // 创建一个缓存线程池（示例接口）
    ThreadPond::CachedThreadPool pool;
    pool.submit([]{ /* 任务 */ });
    pool.shutdown();
}
```

注意：如果项目中没有某些实现，README 作为占位说明；我可以根据实际头文件补全接口说明。
