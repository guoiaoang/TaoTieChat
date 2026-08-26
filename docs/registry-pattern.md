# 注册表模式 (Registry Pattern)

> 参考链接：
>
> [C++ 工厂模式 | Irgb](https://irgb.github.io/C++/%20C++工厂模式/)
>
>[C++11 实现一个自动注册的工厂 - qicosmos(江南) - 博客园](https://www.cnblogs.com/qicosmos/p/5090159.html)
>
>[YaoXuanZhi/CAutoRegisterFactory: 一个支持自动注册的工厂类模板，参考 SOUI（控件注册机制、事件订阅机制）和 Sigslot.h 实现](https://github.com/YaoXuanZhi/CAutoRegisterFactory)

结合了单例模式与工厂模式，实现了不修改工厂类而自动新增并调用不同实现的方法。

## 示例 1：静态声明自动注册类

> 来源：Google Search AI 模式

**插件化工厂 (Pluggable Factory)** 或 **反射式工厂**，结合了 **工厂模式** 与 **单例模式**/**静态注册**。

这种模式的核心思想是：不再在工厂类中写死 `switch-case` 或 `if-else`，而是让具体产品在启动时主动把自己“挂载”到工厂的映射表（Map）中。

为什么这种模式更好？
  1. 符合开闭原则 (OCP)：当你需要增加一种新形状时，只需增加新的 `.cpp` 文件，不需要修改工厂类。
  2. 解耦：工厂完全不知道有哪些具体类存在，它只维护一个注册表。
  3. 动态检测：你可以通过字符串、类型 ID 或从配置文件读取的参数来动态创建对象。

进阶优化建议
  * 宏简化：为了代码整洁，通常会把注册行包装成宏，如 `REGISTER_SHAPE(Circle, "circle")`。
  * 多线程安全：如果注册发生在运行时（而非启动时），需要为 `getRegistry()` 增加互斥锁。
  * 防止符号剥离：在链接静态库时，编译器可能会因为“没人调用这些静态变量”而将其优化掉，需要注意链接参数（如 `/OPT:NOREF` 或 `__attribute__((used))`）。

### 示例 1：静态声明自动注册类 - 代码实现

核心实现方案：注册表模式 (Registry Pattern)

这种方案利用 C++ 静态成员变量在 main 函数执行前初始化的特性完成自动注册。

1. 基本结构

```c++
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <memory>

// 抽象产品
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() = 0;
};

// --- 自动注册工厂 ---
class ShapeFactory {
public:
    // 注册函数类型：接受类型名，返回对象指针
    using Creator = std::function<std::unique_ptr<Shape>()>;

    // 提供给具体产品的注册接口
    static void registerType(const std::string& name, Creator creator) {
        getRegistry()[name] = creator;
    }

    // 使用时检测类型并自动调用
    static std::unique_ptr<Shape> create(const std::string& name) {
        auto& reg = getRegistry();
        if (reg.find(name) != reg.end()) {
            return reg[name](); // 调用注册好的构造函数
        }
        return nullptr;
    }

private:
    static std::map<std::string, Creator>& getRegistry() {
        static std::map<std::string, Creator> instance;
        return instance;
    }
};
```

2. 自动注册辅助类

为了实现“自动”，我们通常定义一个辅助类，利用它的构造函数来调用 registerType。

```c++
template<typename T>
class ShapeRegister {
public:
    ShapeRegister(const std::string& name) {
        ShapeFactory::registerType(name, []() {
            return std::make_unique<T>();
        });
    }
};
```

3. 具体产品实现

在实现具体类时，只需一行代码即可完成“类型注册”，无需修改工厂源代码。

```c++
class Circle : public Shape {
public:
    void draw() override { std::cout << "画一个圆\n"; }
};

// 关键点：定义一个静态全局变量，程序启动时自动触发注册
static ShapeRegister<Circle> regCircle("circle");

class Square : public Shape {
public:
    void draw() override { std::cout << "画一个正方形\n"; }
};
static ShapeRegister<Square> regSquare("square");
```

4. 使用示例

```c++
void draw_shape() {
    std::unique_ptr<Shape> shape_circle = ShapeFactory::create("circle");
    shape_circle->draw();
    std::unique_ptr<Shape> shape_square = ShapeFactory::create("square");
    shape_square->draw();
}
```

## 示例 2：保存创建函数

> 来源：[C++ 工厂模式 | Irgb](https://irgb.github.io/C++/%20C++工厂模式/)

需要注意的是：
   1. 下面的代码中 Factory 只能注册 Base 的子类，如果要实现通用的 Factory，可以把 Factory 改成类模板。
   2. 这里的实现和 [Compile-Time Plugin System(github)] 中的实现是一致的，对应关系为：
      * Factory <=> PluginFactory
      * CreatorFunc <=> IPluginRegistrar
      * Base <=> IPlugin
      * Plugin1, Plugin2 <=> Plugin1

[Compile-Time Plugin System(github)]: https://gist.github.com/Cilyan/a8117124b04b64642646

* main.cpp

```c++
#include "base.h"
#include "factory.h"

using namespace std;
//before entering main function, all global (static) variables will be initialized, in which process plugins are registered to factory.
int main() {
    Factory & factory = Factory::Instance();
    //p1, p2 are unique_ptr<Base>, so caller own new created plugin object.
    auto p1 = factory.Create("Plugin1");
    auto p2 = factory.Create("Plugin2");
    auto p3 = factory.Create("Plugin3");
    if (p1) {
        p1->do_something();
    }
    if (p2) {
        p2->do_something();
    }
    if (p3) {
        p3->do_something();
    }
}
```

* factory.h

```c++
#ifndef _FACTORY_H
#define _FACTORY_H

#include <functional>
#include <string>
#include <unordered_map>

#include "base.h"

class Factory {
public:
    static Factory& Instance() {
        static Factory singleton;
        return singleton;
    }

    typedef std::function<Base*()> CreatorFunc;
    // ATTENTION: can not use register as function name, becase register is a keyword in C++11
    template<typename Derived>
    bool Register(const std::string & plugin_name) {
        CreatorFunc creator_func = Creator<Derived>;
        auto ret = _map.insert(std::make_pair(plugin_name, creator_func));
        return ret.second;
    }

    // do not return raw pointer directly, use smart pointer
    std::unique_ptr<Base> Create(const std::string & plugin_name) {
        auto iter = _map.find(plugin_name);
        if (iter == _map.end()) return std::unique_ptr<Base>(nullptr);
        return std::unique_ptr<Base>((iter->second)());
    }
private:
    template<typename Derived>
    static Derived* Creator() {
        return new Derived();
    }

    std::unordered_map<std::string, CreatorFunc> _map;
    Factory() {}

    // forbid copy constructor and assign operation,
    // to make sure Factory is a Singleton
    Factory(const Factory &) = delete;
    Factory & operator = (const Factory &) = delete;
    // forbid move constructor and move assign operation,
    // to make sure Factory is a Singleton
    Factory(Factory &&) = delete;
    Factory & operator = (Factory &&) = delete;
};

// macro is used widely in many factory pattern implementation. But, macro is ugly, it make program hard to comprehend and debug, so avoid using it
#define REGISTER_PLUGIN(CLASSNAME) \
    static const auto CLASSNAME##register_result = Factory::Instance().Register<CLASSNAME>(#CLASSNAME); \

#endif
```

* base.h

```c++
#ifndef _BASE_H
#define _BASE_H
class Base {
public:
    virtual void do_something() = 0;
};
#endif
```

* plugin1.cpp

```c++
#include <iostream>
#include "base.h"
#include "factory.h"

class Plugin1 : public Base {
    void do_something() {
        std::cout << "Plugin1" << std::endl;
    }
};
//static const auto register_result1 = Factory::Instance().Register("Plugin1", Creator<Plugin1>);
REGISTER_PLUGIN(Plugin1);
```

* plugin2.cpp

```c++
#include <iostream>
#include "base.h"
#include "factory.h"

class Plugin2 : public Base {
    void do_something() {
        std::cout << "Plugin2" << std::endl;
    }
};
//static const auto register_result2 = Factory::Instance().Register("Plugin2", Creator<Plugin2>);
REGISTER_PLUGIN(Plugin2);
```

* 编译运行：
```sh
g++ -std=c++11 main.cpp plugin1.cpp plugin2.cpp -o main
./main
```

## 示例 3：扩展 Factory

* 自动注册的对象工厂的实现思路如下：
  1. 提供一个单例工厂对象。
  2. 工厂注册对象（保存创建对象的 key 和构造器）。
  3. 利用辅助类，在辅助类对象的构造过程中实现目标对象地注册。
  4. 利用一个宏来生成辅助对象。
  5. 在派生类文件中调用这个宏实现自动注册。

其中，需要注意的是，对象工厂并不直接保存对象，而是对象的构造器，因为对象工厂不是对象池，
是对象的生产者，允许不断地创建实例，另外，这样做还实现了延迟创建。
另外一个要注意的地方是借助宏来实现自动注册，本质上是通过宏来定义了很多全局的静态变量，
而这些静态变量仅仅是为了实现自动注册，并没有实际的意义。

* 总结：
  * 使用 C++11，仅仅需要几十行代码就可以实现一个自动注册的对象工厂，
    消除了长长的 swithc-case 语句，还遵循了开闭原则，简洁而优雅。
  * 完整的代码：https://github.com/qicosmos/cosmos/tree/master/self-register-factory
  * 如果都是 hpp 的消息是没问题的，如果是 h 和 cpp 分开的那种，
    多个 cpp 包含含静态变量的头文件会引起的链接问题，这就把静态变量干掉，
    可以参考这个实现：https://github.com/qicosmos/cosmos/blob/master/self-register-factory/MessageFatory1.hpp

* Factory 最终的实现

```c++
#include <map>
#include <string>
#include <functional>
#include <memory>
#include "Message.hpp"

struct factory {
    template<typename T>
    struct register_t {
        register_t(const std::string& key) {
            factory::get().map_.emplace(key, [] { return new T(); });
        }

        template<typename... Args>
        register_t(const std::string& key, Args... args) {
            factory::get().map_.emplace(key, [&] { return new T(args...); });
        }
    };

    static Message* produce(const std::string& key) {
        if (map_.find(key) == map_.end())
            throw std::invalid_argument("the message key is not exist!");

        return map_[key]();
    }

    static std::unique_ptr<Message> produce_unique(const std::string& key) {
        return std::unique_ptr<Message>(produce(key));
    }

    static std::shared_ptr<Message> produce_shared(const std::string& key) {
        return std::shared_ptr<Message>(produce(key));
    }

private:
    factory() {};
    factory(const factory&) = delete;
    factory(factory&&) = delete;

    static factory& get() {
        static factory instance;
        return instance;
    }

    static std::map<std::string, std::function<Message*()>> map_;
};

std::map<std::string, std::function<Message*()>> factory::map_;

#define REGISTER_MESSAGE_VNAME(T) reg_msg_##T##_
#define REGISTER_MESSAGE(T, key, ...) static factory::register_t<T> REGISTER_MESSAGE_VNAME(T)(key, ##__VA_ARGS__);
```

* 示例

```c++
class Message {
public:
    virtual ~Message() {}
    virtual void foo() {}
};
#include "MessageFactory.hpp"
#include "Message.hpp"

class Message1 : public Message {
public:

    Message1() {
        std::cout << "message1" << std::endl;
    }

    Message1(int a) {
        std::cout << "message1" << std::endl;
    }

    ~Message1() {}

    void foo() override {
        std::cout << "message1" << std::endl;
    }
};

//REGISTER_MESSAGE(Message1, "message1", 2);
REGISTER_MESSAGE(Message1, "message1");

#include "Message1.hpp"

int main() {
    Message* p = factory::produce("message1");
    p->foo();   //Message1

    auto p2 = factory::produce_unique("message1");
    p2->foo();
}
```
