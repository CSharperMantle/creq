# creq ![CMake](https://github.com/CSharperMantle/creq/workflows/CMake/badge.svg)

**An elegant way to generate HTTP/1.x messages in C**

[creq](https://github.com/CSharperMantle/creq) is a library written in pure C, which allows you to create HTTP/1.x message strings at your own wishes with easy-to-use object-like APIs. With high performance and adjustable memory usage, creq can be used in low-profile MCUs, such as Arduino(AVR) and ESP-series.

## Features & TODOs

- [x] Customizable request/response generation
- [x] Safe & tweakable memory management
- [x] Great portability
- [x] User-friendly object-like interface
- [ ] Integrated message syntax validator

## Symbol accessibility

When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention. For windows you have 3 define options:
* `CREQ_HIDE_SYMBOLS` - Define this in the case where you don't want to ever `dllexport` symbols
* `CREQ_EXPORT_SYMBOLS` - Define this on library build when you want to `dllexport` symbols (default)
* `CREQ_IMPORT_SYMBOLS` - Define this if you want to `dllimport` symbol

For Unix builds that support visibility attribute, you can define similar behavior by setting default visibility to hidden by adding
* `-fvisibility=hidden` (for gcc)
* `-xldscope=hidden` (for sun cc)

to `CFLAGS`, then using the `CREQ_API_VISIBILITY` flag to "export" the same symbols the way `CREQ_EXPORT_SYMBOLS` does

## License

```plain
Copyright (c) 2020 Bao "Mantle" Rong

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see [LICENSE](LICENSE) .
```
