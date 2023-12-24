# tinyThread

## 基于stm32定时器实现的多线程demo
结合所学的中断上下文知识，在定时器中断中进行保护现场，切换上下文，切换线程，从而实现简单的单核多线程demo

## How to use
```
void *helloworld(void *args)
{
	while (1)
	{
        printf("hello world\n");
	}
}

int main(void)
{
	int thd1 = tinythread_create(helloworld, NULL);
	int thd2 = tinythread_create(helloworld, NULL);
	tinythread_run();
}
```

## todo:
1、创建线程时传递参数存在问题

2、detach函数 join函数待实现