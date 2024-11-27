# FYP-IERG4998-Hoover
Sem1

For our coding 改進方向：

1. 使用 rand() 生成源端口
使用 rand() 来随机选择源端口。这是可以的，但最好加个 srand(time(0)); 来保证每次运行程序时随机数种子的不同。

2. inet_addr() 已弃用
inet_addr() 被认为是过时的函数。虽然它在一些旧的代码中仍然被使用，但现在推荐使用 inet_pton() 来处理 IP 地址的转换。inet_pton() 支持更多的地址格式，且更加安全。

3. sleep() 延迟
你在 while 循环中使用 sleep(1) 来每秒发送一次数据。对于简单的程序这是有效的，但如果程序需要更高的精度或更复杂的定时控制，你可以考虑使用 usleep() 或 nanosleep() 来精确控制延时。


