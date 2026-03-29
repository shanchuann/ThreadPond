# ThreadPond 线程池塘

ThreadPond 实现了一系列简洁且高性能的 C++ 线程池，方便并发任务调度与资源管理，包含固定大小线程池、缓存线程池、工作窃取线程池、定时器线程池、同步队列及定时器队列组件，适用于各种应用场景。

**主要特性**
- 轻量级、现代 C++（C++17）实现
- 支持固定大小线程池（任务提交、优雅关闭）
- 高效的同步队列（阻塞/非阻塞操作）
- 包含示例与测试用例，易于集成与验证

**目录概览**

项目内主要模块与典型文件布局（实际文件/子模块可能更多）：

```
ThreadPond/
├── CMakeLists.txt                // 顶层 CMake 构建配置
├── LICENSE                       // 许可证文件（MIT）
├── README.md                     // 项目说明文档
├── bin/                          // 可能的运行/示例二进制
├── build/                        // 构建输出（本地）
├── FixedThreadPool/              // 固定大小线程池实现
│   ├── include/
│   │   └── FixedThreadPool.hpp
│   └── src/
├── CachedThreadPool/             // 可扩展/缓存线程池实现
│   ├── include/
│   │   └── CachedThreadPool.hpp
│   └── src/
├── WorkStealingPool/             // 工作窃取线程池（可选）
│   ├── include/
│   │   └── WorkStealingPool.hpp
│   └── src/
├── ScheduledThreadPool/          // 定时/调度线程池实现
│   ├── include/
│   │   └── ScheduledThreadPool.hpp
│   └── src/
├── TimerQueue/                   // 定时器队列实现（Timer/TimerQueue）
│   ├── include/
│   │   └── TimerQueue.hpp
│   └── src/
├── logsys/                       // 简单日志系统（可独立使用）
│   ├── include/
│   │   └── Logger.hpp
│   └── src/
└── test/                         // 测试与示例代码
	├── CMakeLists.txt
	├── CachedTest.cpp
	├── FixedTest.cpp
	├── ScheduledTest.cpp
	└── WorkStealingTest.cpp

```

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

2. 运行单元/示例二进制：

```bash
# 如果构建产物放在 bin/ 下
./bin/test
```

在运行测试前，请确保已正确设置 CMakeLists.txt 以包含测试源文件，并启用了测试构建。

**示例用法**

参考头文件 `FixedThreadPool/include/FixedThreadPool.hpp` 中的接口。例如：

```cpp
#include "FixedThreadPool/FixedThreadPool.hpp"

// 创建一个包含 4 个工作线程的线程池
int main() {
	#include <iostream>
	shanchuan::FixedThreadPool pool(4);

	// 提交返回值任务并等待结果
	auto fut = pool.submit([](int a, int b) {
		return a + b;
	}, 2, 3);

	std::cout << "sum = " << fut.get() << std::endl;

	// 提交无返回值任务
	pool.submit([]{
		std::cout << "Hello from worker thread" << std::endl;
	});

	// 直接添加任务（无 future）
	pool.add_task([]{
		// 执行简单任务
	});

	// 优雅关闭线程池，等待所有任务完成
	pool.stop();
	return 0;
}
```

**许可证**

本项目采用 MIT 许可证，详见根目录 `LICENSE`，欢迎提交 issue 与 PR。
