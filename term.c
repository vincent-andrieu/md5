#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

bool select_canonical_mode(bool enable)
{
    static struct termios old;
    static struct termios new;

    if (enable)
    {
        if (ioctl(0, TCGETS, &old) == -1 || ioctl(0, TCGETS, &new) == -1)
            return false;
        new.c_lflag &= ~ECHO;
        new.c_lflag &= ~ICANON;
        if (ioctl(0, TCSETS, &new) == -1)
            return false;
    }
    else
    {
        if (ioctl(0, TCSETS, &old) == -1)
            return false;
    }
    return true;
}

char blocked_read()
{
    char buffer[1];
    ssize_t size;

    do
    {
        size = read(STDIN_FILENO, buffer, sizeof(char));
    } while (size == -1);

    if (size == 0)
        return 0;
    return buffer[0];
}