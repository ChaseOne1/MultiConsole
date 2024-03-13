# MultiConsole

## 项目概述
MultiConsole是一个旨在为Windows平台下C++项目开发提供更清晰的控制台调试支持的开源静态库。库向用户提供便捷的控制台调试输出功能，支持手动/自动控制多个控制台窗口的生命周期和输出视觉丰富的调试信息。未来可能会支持面向服务器环境的网络远程调试功能。

## 开源声明
本项目遵守MIT开源协议，协议声明已随文附上。

## 构建库
本项目使用C++17标准与`CMake Tools(minium version 3.1)`组织构建，建议使用`x86 msvc`编译。  
项目构建还使用到了[Windows PowerShell](https://learn.microsoft.com/en-us/powershell)脚本完成部分功能，如遇到“resource.hpp”相关错误请检查计算机[有关Windows PowerShell脚本的权限](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_execution_policies)。
1. 进入项目根文件夹输入命令，可选择是否使用UNICODE编码格式编译（以不使用为例）
```cmake
cmake -S . -B build -A Win32 -DUNICODE=OFF
```
2. 确定目标编译模式Debug or Release，替换命令中对应部分（以Release为例）
```cmake
cmake --build build --config release
```
3. 将生成的MultiConsole.lib与src/CConsole.hpp拷贝至欲部署项目的合适位置并配置使用

## 使用
本节只阐述基本用法，各内置命令详见[Commands Description](Commands%20Description.md).  
### 创建和销毁控制台窗口  
库中映射到单个控制台窗口的基本对象类型是`CConsole`类，因此每种对CConsole类的实例化语句均可创建出一个新的控制台窗口，所有对CConsole类型对象的释放操作均可销毁一个已存在的控制台窗口。
```cpp
void Foo()
{
    CConsole console1; //创建了一个具有自动储存期的CConsole对象
    CConsole* console2 = new CConsole("Hello World");  //假设创建了一个具有动态储存期的CConsole对象
    /*Some code...*/
    delete console2;  //销毁对象console2
    //console1.~CConsole();  //对象console1在函数结束时已被自动清理
}
```
### 向控制台窗口打印信息
库致力于减少用户对本库的学习和使用成本，因此对CConsole对象的操作均尽可能契合于现有的使用习惯，除直接使用SendCommand函数进行指令驱动外，还支持通过类似std::cout的operator<<方法或类似传统C语言标准库的printf方法操作CConsole对象向控制台窗口打印信息。
```cpp
void Foo()
{
    CConsole console;
    
    //使用SendCommand函数进行指令驱动
    console.SendCommand(COMMAND_PRINT, "Hello World!");
    //使用类似std::cout的operator<<方法
    console << "Hello World!" << cs::endl;  //cs为本库命名空间
    console << COMMAND_PRINT << sizeof "Hello World!" << "Hello World!" << cs::ends;  //输出后不换行
    //使用类似传统printf的方法
    console.printf("%s is %d years old", "Paul", 24);
}
```
### 使用命令驱动高级特性
库支持直接使用SendCommand函数发送指令与单个参数，亦支持使用operator<<方法发送指令和多个参数。
```cpp
void Foo()
{
    CConsole console;
    
    //SendCommand清理输出
    console.SendCommand(COMMAND_CLEAR);
    //operator<<关闭窗口
    console << COMMAND_EXIT << cs::ends;  //使用此类方法发送命令时需要使用cs::ends或cs::endl结束指令模式
}
```
### 重定向标准输出到调试窗口
库致力于兼容多种现有的Log库，目前采用重定向标准输出的方案减轻已有项目部署本库的惩罚，支持将现有项目使用std::cout或printf等向stdout输出的内容重定向到调试窗口。  
**受现有设计限制，在重定向之后、恢复重定向之前使用命令驱动的行为是未定义的。**
```cpp
void Foo()
{
    CConsole console;
    
    console.SetAsDefaultOutput();  //重定向stdout
    std::cout << "stdout to the console window" << std::endl;
    printf("stdout to the console window\n");
    console << "console output\n";
    console.printf("console output\n");
    
    console.ResetAsDefaultOutput();  //恢复stdout至重定向前
    std::cout << "stdout to the stdout" << std::endl;
    printf("stdout to the stdout\n");
    console << "console output\n";
    console.printf("console output\n");
}
```
### 使用注意
库区分Unicode与多字符节集版本，**在使用前请选择或编译合适版本并在使用时注意所涉及到的字符（串）编码格式**，保证数据传输完整。  
库为减轻部署负担，使用了*傀儡进程注入技术*；该方案较为敏感，可能会导致安全软件警告，请忽略或暂时关闭安全软件以继续使用本库。库仅对cmd.exe进行注入，无恶意行为，如存在安全疑虑可阅读本库源码。

## 未来可能支持的
这里是一些持续更新的关于本项目的维护计划状态，项目仍有诸多不完善之处，将会在本年度对已确定的条目进行开发上线。
|          条目          |   确定   |    考虑    |   否定    |
| :--------------------: | :------: | :--------: | :-------: |
|    对x64项目的支持     | :smiley: |            |           |
|     对C语言的支持      |          | :thinking: |           |
|    更多的log库支持     |          | :thinking: |           |
|    更全面的线程安全    | :smiley: |            |           |
|     双向控制台调试     |          | :thinking: |           |
| 内置的对format库的支持 |          | :thinking: |           |
|   更低的C++标准支持    |          |            | :pensive: |


## 参考资料
本库在开发时遇到了许多问题，以下文献和项目给予了本库极大的帮助和启发，在此将它们列出并向作者致以至诚的感谢。

1. [Process-Hollowing](https://github.com/m0n0ph1/Process-Hollowing) -Github m0n0ph1
2. [Windows C++ 将外部exe加载到内存中直接运行](https://www.cppblog.com/mybios/archive/2006/11/20/15452.html) -cppblog 李锦俊(mybios)
3. [Pipes (Interprocess Communications)](https://learn.microsoft.com/en-us/windows/win32/ipc/pipes) -Microsoft