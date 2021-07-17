
# vesselhax

WiiU N64 Virtual Console exploit

## Technical explanation

It's an exploit implementation the bug found in 2016 by **yellows8**

It's a stack buffer overflow happening when handling BMFont "pages". The entire block is copied to stack using just the size, without checking the size or the content.

The exploit happens on the default thread of core0

We have access to ``OSCodegenCopy(void* dst, void* src, size_t len)``, so we can just create a new thread on core1 and copy our code to the [**codegen**](https://wiiubrew.org/wiki/Coreinit.rpl#Codegen)

## Warning 

The offset used are only for **Super Mario 64 PAL**

You can add support for a new game by modifying these:

```C
#define ROP_VESSEL_TEXT_OFFSET 0x00800000
#define ROP_VESSEL_DATA_OFFSET 0x00502200

#define ROP_VESSEL_STACK_PTR 0x1768be98
#define ROP_VESSEL_BMF_FILE_PTR 0x38a86c40
#define ROP_VESSEL_COPY_TO_CODEGEN 0x020600B0 + ROP_VESSEL_TEXT_OFFSET
```


## Usage

- Build **vesselhax_code**, you need **devkitPro** and **devkitPPC**, and simply run ``make``, it will build ``vesselhax_code.bin``.

- Create the ``courier12.fnt`` file, by running ``vesselhax_font_patcher.exe``

The font patcher can be built with ``g++ vesselhax_font_patcher.cpp stream.cpp -o vesselhax_font_patcher.exe``

Building on Linux or MacOS should be fine

- Upload the file to your console

For example, for **Super Mario 64 PAL** and **wupserver**:

```py
w.up("C:/Users/Rambo/Desktop/vesselhax/courier12.fnt"
"/vol/storage_mlc01/usr/title/00050000/10199500/content/data/fonts/courier12.fnt")
```

## Credits

- [**yellows8**](https://wiiubrew.org/wiki/Wii_U_System_Flaws#PPC_userland) for finding the exploit
- [**wiiu-env**](https://github.com/wiiu-env/payload_loader) for the main_hook
- [**Rambo6Glaz**](#) for the implementation
