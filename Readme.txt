

Makefile 框图说明：
    1、根目录下的 Makefile 主要负责调用各个子目录下的 Makefile 目标，
            以及提供编译时使用到的相关工具与参数信息
    2、kernel 目录下的 makefile 主要负责 HSunix 内核的编译与链接
    3、mkfs：目录下的 makefile 主要负责磁盘工具的编译，以及磁盘 fs.img 
            文件的生成
    4、user：目录下的 makefile 主要负责管理用户空间文件的编译链接与生成


MAKE 命令使用说明：
    make build： 编译 HSunix 系统内核
    make app：   编译用户空间代码
    make：       执行完整的系统清理、编译、链接与运行流程
    make qemu：  调用 qemu 直接运行生成的 HSunix 系统文件
    make clean： 清除编译时生成的中间文件
    make remove：清除所有生成的文件
    make debug： 用于对内核源代码进行 debug 


文件夹结构体说明：
    kernel：用于存放 HSunix 内核的所有源码，以及编译生成的文件，
            整个系统在内核空间中执行的内容，都存放在该目录
    mkfs:   用于存放生成挂载到 qemu 上的磁盘文件，属于磁盘文件生成
            的工具
    tools： 存放当前这个系统可能用到的工具
    user：  用于存放执行在用户层的源代码，与 kernel 是分隔开编译的，
            在当前目录下编译、链接生成的文件，最终会通过 mkfs 工具写到
            fs.img 磁盘中



