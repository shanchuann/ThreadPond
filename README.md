# ThreadPond

简洁且高性能的 C++ 线程池与同步队列库，用于并发任务调度与资源管理。

**特性**
- **轻量级**: 以现代 C++ 编写，接口简单易用。
- **固定线程池**: 支持固定大小线程池和任务提交与关闭的安全语义。
- **同步队列**: 高效的生产者-消费者队列，实现阻塞和非阻塞操作。
- **可测试**: 包含示例与测试用例（见 `test/`）。

**目录结构**
- `FixedThreadPool/`：线程池与同步队列实现源文件与头文件。
- `test/`：示例与测试用例。
- `CMakeLists.txt`：构建配置。

**依赖**
- C++17 或更高。
- CMake 3.10+（建议 3.15+）。
- 支持 Linux / macOS / Windows（基于 CMake）。

**快速开始**

1. 创建构建目录并编译：

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -- -j
```

2. 运行示例或测试二进制（如果存在）：

```bash
./bin/test1
# 或在 build 目录下直接运行可执行文件
```

**示例用法**

简单伪代码示例：

```cpp
#include "FixedThreadPool/FixedThreadPool.hpp"

// 创建一个包含 4 个工作线程的线程池
ThreadPond::FixedThreadPool pool(4);

pool.submit([]{
	// 执行任务
});

pool.shutdown(); // 等待所有任务完成并关闭线程池
```

（具体接口请参见 `FixedThreadPool/include/FixedThreadPool.hpp`）

**运行测试**

如果项目包含 CTest 支持，可以在 `build` 目录运行：

```bash
ctest --output-on-failure
```

或者直接运行 repository 中的测试二进制：

```bash
./test/test1
```

**贡献**

欢迎提交 issue 与 PR。请遵循以下流程：
- Fork 仓库并创建 feature 分支。
- 添加或更新测试用例。
- 提交描述清晰的 PR。

**许可证**

本项目采用 MIT 许可证，详见项目根目录的 LICENSE 文件。

----

文件: [FixedThreadPool/include/FixedThreadPool.hpp](FixedThreadPool/include/FixedThreadPool.hpp) | 测试: [test/test1.cpp](test/test1.cpp)
