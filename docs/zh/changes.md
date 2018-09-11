<style type="text/css">
h2
{
    text-align: center;
}
</style>
## 发布说明 ##

### Version 0.4.0.3

* Portable software should be able to store all its files in a single removable location such as a memory stick. EncryptPad creates its setting file in `$HOME/.encryptpad`. After this change EncryptPad will check if there is `encryptpad_repository` directory in the application path and use it for settings and key files.
* Botan has been updated to version 2.7.
* Plog has been updated to version 1.1.4.
* Translation to Lithuanian

### 版本 0.4.0.0

* 对大文件的支持。EncryptPad现在可以处理磁盘镜像、备份等类型的数GB大的文件。
* 在文件加密对话框新增了一个进度条与一个取消按钮。
* 可以设置文本编辑器中的制表符尺寸了。
* 将Botan升级到了2.3。
* 将stlplus升级到了3.15。
* OpenPGP功能已被移动到它自己的库libencryptmsg中。
* 带有持续密钥与双重保护的WAD文件格式已发生变动。现在大于64KB的此类文件将不再向下兼容，即无法在0.3.2.5版中打开。单层加密并不带持续密钥的GPG与EPD文件不受影响。

### 版本 0.3.2.5

* 修复问题：如果一个只受密码保护的EPD文件在解密后不超过4个字符，内容会被无视而EncryptPad会生成一个空文件。这是由于EncryptPad期望在文件中找到一个4字节长的IWAD标记。此类文件现在可以被正确打开了。
* 修复问题：当打开一个明文文件而将其加密保存时，加密用参数不是默认值而是上一个加密的文件的参数。
* 修复问题 ：`encryptpad` `file`命令行参数不支持非ASCII字符，由此导致了无法在Windows资源管理器中双击打开一个路径中含有此类字符的文件。
* 在首选项中加入了更多属性来控制默认的加密用参数：密钥文件随机序列长度、密钥文件加密属性、默认文件加密属性（加密方法、字符串转密钥/s2k、迭代、压缩），和在保存时再次询问密码的时机。
* 修复问题：当多个EncryptPad进程存在而首选项被更新时，最后一个被关闭的进程会覆盖其他进程的改动。
* 在文件加密对话框中新增了一个使用EPD或是GPG加密的选项。之前必须通过手动编辑扩展名才能输出为GPG格式。
* 修复问题：盐在加密或解密后被显示在文件加密对话框中，尽管它与下一次加密完全无关。
* 只有256种可能的字符串转密钥（S2K）预设。文件属性对话框现在有一个下拉列表用以选择它们，这样选择最大或最小的可能迭代数更加容易。
* 默认的迭代数已增至1015808。
* 防止了对本地明文密钥文件的密码弹窗。
* 修复问题：打开一个文件后鼠标光标在文本区域会变为箭头形状。现在它已经看起来像应该的那样了。
* 合并了多个语言的EncryptPad可执行文件。现在EncryptPad会自动识别操作系统的语言。这会简化发布管理，尤其对于打包者来说。
* 增加了`--lang`命令行开关来强制以特定语言显示。示例：`encryptpad --lang FR`会展示法语本地化内容。
* EncryptPad图形界面被翻译为中文了。
* README.md被翻译为法语了。
* stlplus从上游同步了更改。现在它是3.13版了。


### 版本 0.3.2.3

* 可以配置字符串转密钥（s2k）迭代数了。现在可以为每个文件使用不同的值，也可以为新文件设置默认值。这个值也会被用作保护密钥文件。
* 支持GPG文件中的时间戳字节。
* 除了macOS版，现在将可执行文件重命名为小写`encryptpad`。
* 改进了英语用户界面的文本一致性。所有`password`改为`passphrase`。
* 翻译为俄语。
* 翻译为法语。
* 增加了`SHA-512`散列函数。
* 命令行：encryptcli曾会在TTY不可用时以区段错误崩溃。现在它会正常地返回一个错误码了。
* 修复问题：在文件加密对话框中，当清空了密码又重新设置时，密码会被无视且文件会以“仅密钥”模式保存。
* 修复问题：在文件加密中，gpg文件曾以`t`标志加密。这导致了`0x0D`字节在解密时被移除，因为它认为文件仅包含文本内容。这个问题在Windows上并未出现。它仅在Linux/Unix版本的EncryptPad上被发现。

### 版本 0.3.2.2

* 修复问题：在文件加密对话框中，当密钥文件的密码输入错误时，并无消息提示用户。
* BAK文件支持。编辑时，bak文件仅会于第一次保存时被创建。
* 更改了新的wad文件中的小节顺序。现在是目录、密钥、载荷了。现存wad文件不受影响。
* encryptcli可以从文件描述符中读取密码了。
* encryptcli现在支持标准输入输出。
* 重新设计了encryptcli的参数。
* 将Botan更新至1.10.12。
* 64位版本。
* 修复了构建权限。
* 将Linux上的构建改为使用g++。
* 构建现在可以在Linux上使用系统库了，包括Zlib和Botan。
* 修复了Fedora 23上的构建。
* 修复了警告

### 版本 0.3.2.1

* 第一个开源版本。
