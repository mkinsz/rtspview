# FFmpeg

## 跨平台

- windows


- android

1. 下载ffmpeg源码
2. 编译环境配置
3. 新建一个build_android.sh

1.ffmpeg configure配置
修改ffmpeg-4.2.2根目录下configure脚本，使用Notepad++编辑器打开configure 文件，搜索CMDLINE_SET，添加命令行参数cross_prefix_clang

```
CMDLINE_SET="
    $PATHS_LIST
    ar
    arch
    as
    assert_level
    build_suffix
    cc
    objcc
    cpu
    cross_prefix
        #添加命令行参数
    cross_prefix_clang
    custom_allocator
"
```

修改编译工具路径设置，搜索 ar_default="${cross_prefix}${ar_default}" , 找到以下代码

```
ar_default="${cross_prefix}${ar_default}"
cc_default="${cross_prefix}${cc_default}"
cxx_default="${cross_prefix}${cxx_default}"
nm_default="${cross_prefix}${nm_default}"
pkg_config_default="${cross_prefix}${pkg_config_default}"
```
复制代码，将中间两行修改为
```
ar_default="${cross_prefix}${ar_default}"
#-------------------修改下面两行-----------------------------
cc_default="${cross_prefix_clang}${cc_default}"
cxx_default="${cross_prefix_clang}${cxx_default}"
#------------------------------------------------------------
nm_default="${cross_prefix}${nm_default}"
pkg_config_default="${cross_prefix}${pkg_config_default}"
```

修改这两处即可，详细描述参考作者“开发的猫”的文章[FFmpeg so库编译](https://juejin.im/post/5e130df55188253a8b42ebac)

windows下编译，根据cpu的架构进行修改

```
#!/bin/bash
set -x
API=28
CPU=x86
OUTPUT=/e/work/ffmpeg-4.2.2/android/$CPU
NDK=/c/Android/android-ndk-r21
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/windows-x86_64
SYSROOT=$TOOLCHAIN/sysroot

function build
{
  ./configure \
  --prefix=$OUTPUT \
  --target-os=android \
  --arch=x86 \
  --cpu=x86 \
  --disable-asm \
  --enable-neon \
  --enable-cross-compile \
  --enable-shared \
  --disable-static \
  --disable-doc \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-symver \
  --disable-ffmpeg \
  --sysroot=$SYSROOT \
  --cross-prefix=$TOOLCHAIN/bin/i686-linux-android- \
  --cross-prefix-clang=$TOOLCHAIN/bin/i686-linux-android$API- \
  --extra-cflags="-fPIC"

  make clean
  make -j4
  make install
}

build
```

## 问题

1. ffmpeg安卓x86平台编译错误(libavcodec.so: has text relocations)
原因是android 6.0之后，系统做了限制。
```
On previous versions of Android, if your app requested the system to load a shared library with text relocations,
the system displayed a warning but still allowed the library to be loaded. Beginning in this release, the system
rejects this library if your app’s target SDK version is 23 or higher. To help you detect if a library failed to load,
your app should log the dlopen(3) failure, and include the problem description text that the dlerror(3) call returns.
To learn more about handling text relocations, see this guide
```
针对这个说法，网上很多朋友用降低targetSDK版本的方式来解决，但如果不降低如何解决呢？
官方描述中，x86平台会出现此问题，而armv7-a 、x86_64都没有这样的问题，所以只需要针对x86平台解决即可。
OK，可以通过编译时添加 **–disable-asm**参数解决，设置参数关闭汇编后，编译后.so文件就不存在text relocations了。

可以通过以下方式检查，so文件中是否存在text relocations:
readelf -a path/to/yourlib.so | grep TEXTREL
如果存在文本重定向的话，它就会显示类似的信息：
0x00000016 (TEXTREL)                    0x0

以下是笔者查询的时候出现的信息：
```
MSYS /c/Android/android-ndk-r21/toolchains/x86-4.9/prebuilt/windows-x86_64/bin
$ ./i686-linux-android-readelf.exe -a /e/work/code/rtsp/tview/ffmpeg/android/lib/libavcodec.so | grep TEXTREL
0x00000016 (TEXTREL)                    0x0
0x0000001e (FLAGS)                      SYMBOLIC TEXTREL BIND_NOW
```

添加-disable-asm之后，重新编译, 如果grep不到TEXTREL
```
MSYS /c/Android/android-ndk-r21/toolchains/x86-4.9/prebuilt/windows-x86_64/bin
$ ./i686-linux-android-readelf.exe -a /e/work/ffmpeg-4.2.2/android/x86/lib/libavcodec.so | grep TEXTREL

MSYS /c/Android/android-ndk-r21/toolchains/x86-4.9/prebuilt/windows-x86_64/bin
$

```

## 参考

- 编译参考
[FFmpeg so库编译](https://juejin.im/post/5e130df55188253a8b42ebac#heading-11)

- 问题参考：
(ffmpeg安卓x86平台编译错误)[https://blog.csdn.net/marco_0631/article/details/73292199?utm_source=blogxgwz1]
(Detected problems with app native libraries (please consult log for detail): lib.so: text relocation)[https://blog.csdn.net/quantum7/article/details/104910168/]
