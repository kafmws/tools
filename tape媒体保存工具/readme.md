### 批量保存自己动态中的媒体数据

从[链接](https://activity.askbox.ink/export.html)可以导出动态数据

但图片、音视频还是从`tape`服务器访问的，也就是说媒体数据仍有无法访问的风险

本脚本将图片、音频、视频批量保存到本地

### 使用方法

（所提供exe程序目前仅支持Windows，其它平台请考虑直接使用脚本）

1. 解压导出的`zip`文件，并将解压出的文件夹重命名为`tape`
2. 将`exe`程序与数据文件夹`tape`放在同一级目录下
例如 `D:/aaa/bbb/tape` 和 `D:/aaa/bbb/tape_snapshot.exe`
3. 双击运行`tape_snapshot.exe`，按照提示使用

程序会生成新的文件夹`tape_local`，网页中的媒体资源将访问本地保存的文件而不是`tape`服务器

---

祝使用愉快。

---

程序出错可联系 wdxdesperado at qq dot com
不保证回复