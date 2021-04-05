# Library overview

## libc

This library is the main library that the kernel and userspace uses. It's userspace/kernelspace agnostic using the compilation flag `-DKERNEL` or `-DUSER`.

The libc implementation in Melvix is different to every other libc you've probably seen. It doesn't comply with any GNU/POSIX/Unix/Linux/whatever standard at all, and that's intentional: As the syscalls and general kernel-userspace ABI is completely different than the one that e.g. Unix/Linux/Windows is using, it would be nearly impossible to use a standard libc layout (for example, Melvix doesn't have `open`, `close` or `fork` syscalls). I also believe that the libc standard is way too complicated and enrooted in ancient decisions that don't make sense today at all. Furthermore, the goal of Melvix isn't the support of third party software but on the contrary the creation of a modern and new standard for OS hobbyists. I'm well aware though, that some decisions I chose are probably completely brainlessly stupid, so feel free to propose changes to the libc layout using a pull request or issue.

This libc is therefore constantly changing and has no support for older versions at all. Don't expect any stability in the near future.

Other than that, Melvix' libc has many features that glibc, for example, has too. All the `str*` functions like `strlen` or `strlcpy` are located in the accordingly named `str.c` file with its header `str.h`. In this instance you can already see the changes I've previously talked about: Melvix does not support the almost always memory-unsafe `strcpy` and replaces it with `strlcpy`.

Feel free to write more documentation here, but I believe that most features are self-explanatory if you look through the files in the `inc/` directory (they are quite small and comprehensible).

## libgui

This library is only usable in userspace and is the main GUI framework. This library includes the graphical drawing algorithms and widget wrappers (for example buttons) as well as window manager API connections and image/font loading. It also suffers from performance and security issues and is constantly changing for that reason.

## libnet

Libnet implements some basic userspace networking features such as IP/DNS/HTTP that interact with the kernel network implementations. It's not nearly finished and will change a lot in the future.

## libtxt

The main purpose of libtxt is the parsing of several file formats such as XML/HTML/JSON. Many features will be added in the future and bigger parsing implementations could be moved into a new library for that specific purpose.
