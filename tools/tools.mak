#
# 存放一些常用的 makefile 指令
#


all-type-files-in = $(shell ls $(1) -R | grep '^\./.*:$$' | awk '{gsub(":","");print}') .  


# 获取指定路径下的所有指定类型的文件
# $(call all-c-files, <dir>)
# $(call all-h-files, <dir>)
all-java-files = $(wildcard $(1)*.jave)
all-c-files    = $(wildcard $(1)/*.c)
all-cpp-files  = $(wildcard $(1)/*.cpp)
all-h-files    = $(wildcard $(1)/*.h)
all-S-files    = $(wildcard $(1)/*.S)
all-o-files    = $(wildcard $(1)/*.o)


# 遍历指定路径下的所有子类文件夹下的指定文件
# $(call all-c-files-in, <dir>)
# $(call all-h-files-in, <dir>)
all-java-files-in = $(foreach n,$(call all-type-files-in, $(1)) , $(wildcard $(n)/*.java))
all-c-files-in    = $(foreach n,$(call all-type-files-in, $(1)) , $(wildcard $(n)/*.c))
all-cpp-files-in  = $(foreach n,$(call all-type-files-in, $(1)) , $(wildcard $(n)/*.cpp))
all-h-files-in    = $(foreach n,$(call all-type-files-in, $(1)) , $(wildcard $(n)/*.h))
all-S-files-in    = $(foreach n,$(call all-type-files-in, $(1)) , $(wildcard $(n)/*.S))
all-o-files-in    = $(foreach n,$(call all-type-files-in, $(1)) , $(wildcard $(n)/*.o))
