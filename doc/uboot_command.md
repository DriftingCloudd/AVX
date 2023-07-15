mmc list
查看mmc列表

mmc dev

mmc part

ls mmc1:1

printenv查看环境变量

printenv variable
打印具体某个环境变量

setenv variable
设置某个环境变量

saveenv保存环境变量

setenv ipaddr 192.168.120.230;setenv serveraddr 192.168.120.10;tftpboot 0x80200000 kernel.bin;go 0x80200000