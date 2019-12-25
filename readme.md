# readme

17373181_王适意

## 如何编译和使用

### 编译

```shell
# build_project.sh
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make
```



### 使用

```
Usage:
  cc0 [options] input [-o file]
or 
  cc0 [-h]
Options:
  -s        将输入的 c0 源代码翻译为文本汇编文件
  -c        将输入的 c0 源代码翻译为二进制目标文件
  -h        显示关于编译器使用的帮助
  -o file   输出到指定的文件 file

不提供任何参数时，默认为 -h
提供 input 不提供 -o file 时，默认为 -o out
```



## 完成部分

- 基础 C0
- 注释
- 字符字面量与字符串字面量
  - char 字面量可以参与表达式运算
- 作用域与生命周期



## 对文法/语义规则的等价改写

等价改写后的文法请见 `/refer/c0-grammar-alter.txt`



## 对未定义行为的定性

1. 字面量溢出

> UB: 虽然字面量有类型意味着其有着受限的值域，但是C0并不要求对溢出进行报错，使用过大的字面量是未定义行为。

遇到超出 int 阈值范围的整数字面量报出编译错误 `ErrIntegerOverflow`



2. 关系表达式的值

> UB:  关系表达式`<condition>`在这里没有规定实际类型，只说明了什么样的情况应该视为true或false，但是从未说明其本身的值应该是多少（即没有说必须是0或1）

因为它不出现在赋值语句右侧以及函数传参，因此这里不进行强制约束



3. const 变量初值

> UB: 未初始化的非`const`变量，C0不要求对其提供默认值，因此使用它们是未定义行为。

const 变量未初始化报出编译错误 `ErrConstantNeedValue`



4. 控制流返回

> UB: 函数中的控制流，如果存在没有返回语句的分支，这些分支的具体返回值是未定义行为

- 返回值不为 void 的函数，如果有控制流分支末尾没有返回，报出编译错误 `ErrNeedReturnValue`
- 返回值为 void 的函数执行到控制流末尾自动返回



5. 输入输出

> UB: scan和print是目标机有关的内容，当I/O流出现问题时，它们的表现是未定义的

使用助教提供的虚拟机