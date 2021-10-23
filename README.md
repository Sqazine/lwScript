

# lwScript

轻量的脚本语言实现,具体实现教程见[这里]

<!-- PROJECT SHIELDS -->

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![Apache-2.0 License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />

## 环境

1. C++ 17编译器
2. CMake(>=3.10)

## 运行

1. 编译

```sh
git clone https://github.com/Sqazine/lwScript.git
cd lwscript
mkdir build
cd build 
cmake ..
cmake -build .
```

2. 命令行运行
```sh
window:
    .\lwScript.exe  
    >let a=10;println("{}",a);


linux:
    chmod 777 lwscript
    ./lwScript 
    >let a=10;println("{}",a);  
```
3. 源码文件运行
```sh
window:
    .\lwScript.exe examples/leetcode-twosum.lws


linux:
    chmod 777 lwscript
    ./lwScript examples/leetcode-twosum.lws
```

## 例子
1. 变量声明
```sh
let a=10;
let b=a;
let c;
b=20;
println("{}",a);#10
println("{}",b);#20
println("{}",c);#"nil"
```
2. 函数
```sh
function add(x,y){
    return x+y;
}

let c=add(1.000000,2.000000);
println("{}",c);#3.000000

#原生函数
println("{}","hello world!");#输出到控制台的函数

let a=[1,2,3];#数组
sizeof(a);#获取数组个数
```
3. 数组
```sh

function add(vec1,vec2){
    return [vec1[0]+vec2[0],vec1[1]+vec2[1]];
}

function sub(vec1,vec2){
    return [vec1[0]-vec2[0],vec1[1]-vec2[1]];
}

let vec1=[3,3];
let vec2=[2,2];

let vec3=add(vec1,vec2);

println("{}",vec3);#[5.000000,5.000000]
```

4. 条件
```sh
let a=10;
let b=a;
b=20;
println("{}",a);#10.000000
println("{}",b);#20.000000

if(b<a)
    b=a;
else 
{
    a=b;
}

println("{}",a);#20.000000
println("{}",b);#20.000000


if(a>100)
    a=100;
else if(a>50)
    a=50;
else if(a>30)
    a=30;
else a=5; 

println("{}",a);#5.000000

if(b>a)
   if(a==100)
        a=1000;
    else 
        a=500;
else 
    a=300;

println("{}",a);#500.000000
```

5. 循环
```sh
let a=0;

while(a<100)
{
    println("{}",a);
    a=a+1;
}

# 0.000000
#...
#...
#...
# 99.000000
```

6. 结构体
```sh
struct Vec2
{
    let x=0;
    let y=0;
}
struct Vec3
{
    let vec2=Vec2;
    let z=0;
}
struct Vec4
{
    let vec3=Vec3;
    let w=0;
}
let a=Vec4;
a.vec3.vec2.x=1000;
println("{}",a);
#struct
#{
#    vec3=struct
#         {
#            z=0.000000
#           vec2=struct
#                 {
#                    x=1000.000000
#                    y=0.000000
#                 }
#         }
#   w=0.000000
#}
println("{}",a.vec3.vec2.x);# 1000.000000
```

7. 链表模拟
```sh
struct Node
{
    let v=0;
    let next=nil;
}

let head=Node;

let e=head;
let i=1;
while(i<10)
{
    let e2=Node;
    e2.v=i;

    e.next=e2;

    e=e.next;

    i=i+1;
}

println("{}",head);

#struct{
#    v=0.000000
#    next=struct
#        {
#            v=1.000000
#            next=struct
#                {
#                    v=2.000000
#                    next=struct
#                        {
#                            v=3.000000
#                            next=struct
#                                {
#                                    v=4.000000
#                                    next=struct
#                                        {
#                                            v=5.000000
#                                            next=struct
#                                                {
#                                                    v=6.000000
#                                                    next=struct
#                                                        {
#                                                            v=7.000000
#                                                            next=struct
#                                                                {
#                                                                    v=8.000000
#                                                                    next=struct
#                                                                        {
#                                                                            v=9.000000
#                                                                            next=nil
#                                                                        }
#                                                                }
#                                                        }
#                                                }
#                                        }
#                                }
#                        }
#                }
#        }
#}
```

8. leetcode 两数之和
```sh
let nums=[2,7,11,15];
let target=9;

function twosum(nums,target)
{
    let i=0;
    let j=i+1;
    println("{}",j);

    while(i<sizeof(nums)-1)
    {
        j=i+1;
        while(j<sizeof(nums))
        {
            println("{}",nums[i]);
            println("{}",nums[j]);
            if(nums[i]+nums[j]==target)
                return [i,j];
            j=j+1;
        }
        i=i+1;
    }
}

println("{}",twosum(nums,target));#[0.000000,1.000000]
```

## 特性
1. 基本语法

``` sh
1. 每个语句后跟一个分号';',与C语言类似:
let a=10;

2. 使用 '#'表示单行注释,不支持多行注释:
#let a=10;

3. 标识符以字母 A-Z 或 a-z 或下划线 _ 开始,后跟零个或多个字母,下划线和数字(0-9),不允许出现标点字符,比如 @,$和%,也不允许以字母为开头的标识符:
let _a=10;
let Aa=10;
let 9a=10;#非法,不能以数字作变量名开头
let a;#a默认赋nil值

4.关键字:
let: 声明变量
function: 声明函数
struct: 声明结构体
if: 条件语句
else: 条件语句否定分支(与if一起用,不可单独使用)
while: 循环语句
return: 返回语句(可以带一个或零个参数)
```
2. 变量类型
```sh
  let a=10; 数值类型(使用double的C++类型)
  let a="string"; 字符串类型
  let a=true/false; 布尔类型
  lat a=[1,2,3]; 数组类型
  struct Vec2{let x=0;let y=0} let a=Vec2;结构体类型
```

3. 运算符
```sh
1. 算术运算符:
    a+b;
    a-b;
    a*b;
    a/b;
2. 关系运算符:
    a==b;
    a!=b;
    a>b;
    a>=b;
    a<b;
    a<=b;
3. 逻辑运算符:
    a&&b;
    a||b;
4. 赋值运算符:
    a=b;
```

4. 条件语句(与C语言类似)

```sh
1. 单if语句:
    if(a<b)
        return a;
   或者
   if(a<b)
   {
       return a;
   }

2. if-else语句:
    if(a<b)
        return a;
    else return b;
    或者
    if(a<b)
    {
        return a;
    }
    else 
    {
        return b;
    }

3. if-else if-else语句:
    if(a>10)
        return 10;
    else if(a>5)
        return 5;
    else return 0;

4. dalng-else(else与最接近的if语句匹配):
    if(a<b)
        if(a<10)
            return 10;
        else 
            return a;
    else 
        return b;
```

5. 作用域({}内为一个局部作用域)
```sh
let a=10;#全局变量

{
    let a=100;#局部变量
    println("{}",a);#100
}

println("{}",a);#10
```

6. 循环(仅支持while循环)
```sh
while(true)
{
    #多个执行语句
}
或则和
while(true)
    #单条执行语句
```

7. 函数
```sh
function add(x,y)
{
    return x+y;
}

let a=add(1,2);
```

8. 数组
```sh
let a=[1,2,3];#以[]表示一个数组,内部元素以','分隔
```

9. 数组索引
```sh
let a=[1,2,3];
println("{}",a[0]);#以 数组变量+'['+数值变量+']'表示数组的索引
println("{}",a[10]);#非法的索引会报Index out of array range错误
```

10. 结构体
```sh
struct Vec2
{
    let x=0;
    let y=0;
}

struct Vec3
{
    let vec2=Vec2;#支持结构体嵌套
    let z=0;
}

let a=Vec3;
a.vec2.x=1000;#支持结构体成员赋值
println("{}",a.vec2.x);#支持结构体成员获取值
```

## 版权说明

该项目签署了 Apache-2.0 License 授权许可,详情请参阅 [LICENSE](https://github.com/Sqazine/lwScript/blob/main/LICENSE)

<!-- links -->
[your-project-path]:Sqazine/lwScript
[contributors-shield]: https://img.shields.io/github/contributors/Sqazine/lwScript.svg?style=flat-square
[contributors-url]: https://github.com/Sqazine/lwScript/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/Sqazine/lwScript.svg?style=flat-square
[forks-url]: https://github.com/Sqazine/lwScript/network/members
[stars-shield]: https://img.shields.io/github/stars/Sqazine/lwScript.svg?style=flat-square
[stars-url]: https://github.com/Sqazine/lwScript/stargazers
[issues-shield]: https://img.shields.io/github/issues/Sqazine/lwScript.svg?style=flat-square
[issues-url]: https://img.shields.io/github/issues/Sqazine/lwScript.svg
[license-shield]: https://img.shields.io/github/license/Sqazine/lwScript.svg?style=flat-square
[license-url]: https://github.com/Sqazine/lwScript/blob/master/LICENSE



