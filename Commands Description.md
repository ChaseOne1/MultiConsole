## 命令概述
在项目代码[eConsoleCommands.hpp](src/eConsoleCommands.hpp)中列举了所有可通过SendCommand函数发送的指令。控制台窗口通过接收命令和参数来完成活动，当遇到错误时会在控制台窗口中输出“Console has terminated.”并退出（Debug模式下不会退出）。

## 命令详述
下表展示了所有有效命令及其实际参数，**使用未在表中说明的命令的行为是未定义的。**
|          命令           |                           参数                           |                                                           说明                                                           |
| :---------------------: | :------------------------------------------------------: | :----------------------------------------------------------------------------------------------------------------------: |
|      COMMAND_PRINT      |    parm1:欲打印的字符串字节数<br>parm2:欲打印的字符串    |                                                                                                                          |
| COMMAND_SET_PRINT_COLOR |      parm1: sizeof WORD<br>parm2:WORD类型字符属性值      | 参数在`WinCon.h`中定义，详见[控制台屏幕缓冲区](https://learn.microsoft.com/en-us/windows/console/console-screen-buffers) |
|      COMMAND_CLEAR      |                          无参数                          |                                                    类似system("cls")                                                     |
|  COMMAND_COLORED_CLEAR  |      parm1: sizeof WORD<br>parm2:WORD类型字符属性值      |                                                同COMMAND_SET_PRINT_COLOR                                                 |
|    COMMAND_SET_TITLE    | parm1:欲设置的标题字节数<br>parm2:欲设置的控制台窗口标题 |                                                                                                                          |
|     COMMAND_GOTOYX      |  parm1:sizeof COORD<br>parm2:COORD类型表示的光标坐标值   |                                             COORD的X值为列坐标，Y值为行坐标                                              |
| COMMAND_SET_BUFFER_SIZE |  parm1:sizeof COORD<br>parm2:COORD类型表示的缓冲区大小   |                                                                                                                          |
|     COMMAND_SYSTEM      |        parm1:命令字符串字节数<br>parm2:命令字符串        |                                                    类似system(parm2)                                                     |
|      COMMAND_EXIT       |                          无参数                          |                                                      关闭控制台窗口                                                      |

## 命令使用说明
* 除命令（COMMAND_XXXXX）外，库内部传输数据时首先传输数据字节数，而后传递数据本身；故部分命令参数逻辑上为1个但实际为2个，且按“实际参数-逻辑参数”的顺序声明在上表。   
* 使用SendCommand函数运算符发送非字符串相关指令时，只需传递1个逻辑参数即可，库会根据参数自动计算字节数并发送；使用其发送字符串相关指令时，若参数为字符串字面量时，亦只需传递1个逻辑参数，否则需要使用在[src/CConsole.hpp](src/Console/CConsole.hpp)中定义的字符串字节数计算宏计算字符串字节数并作为SendCommand函数的第三个参数：CALC_STR_BYTES（多字符节集时）、CALC_WSTR_BYTES（Unicode字符集时）、CALC_TSTR_BYTES(自动拓展到合适宏)。  
* 使用operator<<发送命令时，除在发送命令后传递所有的逻辑参数外，需要在命令调用结尾使用cs::ends或cs::endl结束命令模式，否则会导致非预期行为。