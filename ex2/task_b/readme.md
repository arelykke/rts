output with sanitizer:

x ./taskb
Array:{0}
Array:{0, 1}
Array:{0, 1, 2}
Array:{0, 1, 2, 3}
Array:{0, 1, 2, 3, 4}
=================================================================
==58354==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6040000003b8 at pc 0x00010f163caa bp 0x7ff7b0d9e080 sp 0x7ff7b0d9e078
WRITE of size 8 at 0x6040000003b8 thread T0
    #0 0x10f163ca9 in array_insertBack array.c:100
    #1 0x10f161cd7 in main main.c:7
    #2 0x7ff817c3c344 in start+0x774 (dyld:x86_64+0xfffffffffff5c344)

0x6040000003b8 is located 0 bytes after 40-byte region [0x604000000390,0x6040000003b8)
allocated by thread T0 here:
    #0 0x10fc18a20 in wrap_malloc+0xa0 (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0xdca20)
    #1 0x10f161f84 in array_new array.c:12
    #2 0x10f161cb4 in main main.c:4
    #3 0x7ff817c3c344 in start+0x774 (dyld:x86_64+0xfffffffffff5c344)

SUMMARY: AddressSanitizer: heap-buffer-overflow array.c:100 in array_insertBack
Shadow bytes around the buggy address:
  0x604000000100: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 05
  0x604000000180: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 05
  0x604000000200: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 07
  0x604000000280: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 00
  0x604000000300: fa fa 00 00 00 00 00 00 fa fa 00 00 00 00 00 05
=>0x604000000380: fa fa 00 00 00 00 00[fa]fa fa fa fa fa fa fa fa
  0x604000000400: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x604000000480: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x604000000500: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x604000000580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x604000000600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==58354==ABORTING


output without sanitizer:

> ./taskb 
Array:{0}
Array:{0, 1}
Array:{0, 1, 2}
Array:{0, 1, 2, 3}
Array:{0, 1, 2, 3, 4}
Array:{0, 1, 2, 3, 4, 5}
Array:{0, 1, 2, 3, 4, 5, 6}


without the sanitizer, the program wrties to memory which has not been allocated.
this is a critical issue, and can lead to undefined behavior.