//
// Created by jhwangbo on 22.01.19.
//

#ifndef RAISIMOGREVISUALIZER_MISC_HPP
#define RAISIMOGREVISUALIZER_MISC_HPP

#ifdef __unix__
#include <unistd.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

void MSLEEP(int sleepMs)
{
#ifdef __unix__
  usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef WINDOWS
  Sleep(sleepMs);
#endif
}

#endif //RAISIMOGREVISUALIZER_MISC_HPP
