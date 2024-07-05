

#define  N     11            //定义获取的数据个数

void user_space (void)
{
    unsigned short value_buf[N];
    unsigned short i, j, temp;

    while(1)
    {
        /* 添加滤波算法，验证代码在用户层的执行情况 */
        for (i=0; i<N; i++)
        {
            value_buf[i] = i * 5;
        }
        for(j = 0; j < N - 1; j++)
        {
            for(i = 0; i < N - j; i++)
            {
                if(value_buf[i] > value_buf[i + 1])
                {
                    temp = value_buf[i];
                    value_buf[i] = value_buf[i + 1]; 
                    value_buf[i + 1] = temp;
                }
            }
        }
        temp = 0x88;

        /* 执行系统调用，切换到特权模式 */
        asm volatile (
            "ecall"
        );

        /* 验证退出特权模式后，上下文是否保持一致 */
        temp = value_buf[(N - 1) / 2];
    }
}



