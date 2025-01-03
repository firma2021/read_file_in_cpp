# C++ File Reading Methods Benchmark

这个项目演示和比较了 C++ 中不同的文件读取方法及其性能表现。



## 使用方法

编译并运行:
```bash
g++ read_file.cpp -std=c++23 -O3 -o test
./test
```



## 文件读取方法

read_file.cpp中实现了以下函数，它们将文件中的全部内容读取到`string`中，返回`string`。

### 0.使用stdio库的ftell和fseek获取文件大小，而后用fread读取

### 1. 获取文件大小后，使用istream.read读取
- `read_file_using_filesize_and_read`: 使用 `filesystem::file_size` 获取文件大小
- `read_file_using_tellg_and_read`: 使用 `seekg + tellg` 获取文件大小
- `read_file_using_tellg_and_read_and_override`: 使用 C++23 的 `string.resize_and_overwrite` 优化

### 2. 使用流插入/提取运算符，从输入流缓冲区中读取
- `read_file_using_ostringstream`:  `oss << fin.rdbuf(); return std::move(oss).str();`
- `read_file_using_stream_extraction`:   `fin >> noskipws >> oss.rdbuf(); return std::move(oss).str(); `

### 3. 使用输入流缓冲区迭代器逐字节读取
- `read_file_using_iterators`: `return string {istreambuf_iterator{fin}, {}};`





## 性能比较

测试条件：
- 文件大小：100 MB
- 迭代次数：10次

测试结果：
test_file.size(): 100 MB, iter time: 10
file_size + stdio.fread cost: 410 ms
file_size + fin.read cost: 438 ms
tellg + fin.read cost: 395 ms
tellg + fin.read + resize_and_overwrite cost: 397 ms
oss << fin.rdbuf() cost: 700 ms
fin >> noskipws >> oss.rdbuf() cost: 781 ms
istreambuf_iterator cost: 2782 ms



## 结论

1. stdio库不比iostream库快，且需要手动关闭文件，不推荐使用。
2. 方法1性能最佳
3. 方法2性能适中，比方法1慢约1倍
4. 方法3性能最差，比方法1慢约7倍



## 应用场景

一般情况下，使用方法1读取文件

如果无法获取文件的大小（如cin关联的文件），使用方法2读取文件

如果无法获取文件的大小，且要将文件读取到 `vector<char>`等容器而不是`string`，使用方法3读取文件

## 实用函数

对于可获取文件大小的文件，使用方法1读取；否则使用方法2读取。

```cpp
string read_file(istream& in)
{
    in.exceptions(ios::failbit | ios::badbit);

    auto begin_pos = in.tellg();
    if (begin_pos == -1) { return (ostringstream {} << in.rdbuf()).str(); }

    in.seekg(0, ios::end);
    auto end_pos = in.tellg();
    in.seekg(begin_pos);

    string content;
    content.resize(static_cast<size_t>(end_pos - begin_pos));
    in.read(content.data(), static_cast<streamsize>(content.size()));

    return content;
}

string read_file(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    return read_file(fin);
}
```
