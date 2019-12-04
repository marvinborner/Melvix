void write(const char *str, int len)
{
    int ret;
    asm volatile ("push %%ebx; movl %2,%%ebx; int $0x80; pop %%ebx" \
                : "=a" (ret) \
                : "0" (1), "b" ((int) (str)), "c"((int) (len)));
}

void user_main()
{
    const char hello[] = "Switched to usermode!\n";
    write(hello, sizeof(hello));

    while (1) {};
}