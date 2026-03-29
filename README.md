# ThreadPond — 线程池塘

ThreadPond 是一个简洁且高性能的 C++ 线程池与同步队列库，方便并发任务调度与资源管理。

**主要特性**
- 轻量级、现代 C++（C++17）实现
- 支持固定大小线程池（任务提交、优雅关闭）
- 高效的同步队列（阻塞/非阻塞操作）
- 包含示例与测试用例，易于集成与验证

**目录概览**
- `FixedThreadPool/`：线程池与同步队列实现
- `test/`：测试与示例
- `CMakeLists.txt`：构建配置

**依赖**
- C++17 或更高
- CMake 3.10+（建议 3.15+）

**快速开始**

1. 从仓库根目录创建构建目录并编译：

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -- -j
```

2. （可选）运行单元/示例二进制：

```bash
# 如果构建产物放在 bin/ 下
./bin/test1

# 或者直接在 build 目录下运行生成的可执行文件，例如：
./test/test1
```

3. 使用 CTest 运行测试（如果启用了 CTest）：

```bash
ctest --output-on-failure
```

**示例用法**

参考头文件 `FixedThreadPool/include/FixedThreadPool.hpp` 中的接口。例如：

```cpp
#include "FixedThreadPool/FixedThreadPool.hpp"

// 创建一个包含 4 个工作线程的线程池
ThreadPond::FixedThreadPool pool(4);

pool.submit([]{
	// 执行任务
});

pool.shutdown(); // 等待所有任务完成并关闭线程池
```

**开发与贡献**

- 欢迎提交 issue 与 PR。
- 建议流程：Fork → 新分支 → 添加/更新测试 → 提交 PR

**许可证**

本项目采用 MIT 许可证，详见根目录 `LICENSE`。

----

文件: [FixedThreadPool/include/FixedThreadPool.hpp](FixedThreadPool/include/FixedThreadPool.hpp) | 测试: [test/test1.cpp](test/test1.cpp)
