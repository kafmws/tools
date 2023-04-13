### 批量保存自己动态中的媒体数据

从[链接](https://activity.askbox.ink/export.html)可以导出动态数据

但图片、音视频还是从`tape`服务器访问的，也就是说媒体数据仍有无法访问的风险

本脚本将图片、音频、视频批量保存到本地，并生成新的离线网页

### 使用方法

（目前仅提供Windows平台的exe程序，其它平台请考虑直接使用脚本）

1. 将官网导出的`zip`文件重命名为`tape.zip` 或者
  解压官网导出的`zip`文件，将解压出的手机号码文件夹重命名为`tape`

2. 将本`exe`文件与`tape.zip`文件（或解压并重命名的`tape`文件夹）放置于同一目录下
  如：
```
  ────上级目录
    ├─tape                  【若解压则重命名解压出的手机号码文件夹为tape】
    ├─tape.zip              【不解压则重命名zip文件】
    └─tape_snapshot.exe      (本程序)
```
3. 运行本程序，保持网络通畅，等待转存完成


程序会生成新的以`_local`结尾的文件夹，该目录中网页将访问本地保存的媒体文件而不是`tape`服务器

图片/音频/视频分别保存在各个用户目录下的picture/audio/video目录下

转存的结果以`_local`结尾的文件夹可以整体移动，也可以重命名

---

### 脚本使用方法

```
python tape_snapshot.py _xxx.zip_
python tape_snapshot.py _手机号目录_
```

---

祝使用愉快。

---

程序出错可联系 wdxdesperado at qq dot com

不保证回复
