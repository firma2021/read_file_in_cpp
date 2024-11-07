#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace std::chrono;

string read_file_using_filesize_and_read(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    fin.exceptions(ios::failbit | ios::badbit);

    string content;
    content.resize(filesystem::file_size(file_name));
    fin.read(content.data(), static_cast<streamsize>(content.size()));
    return content;
}

string read_file_using_tellg_and_read(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    fin.exceptions(ios::failbit | ios::badbit);

    auto begin_pos = fin.tellg();
    fin.seekg(0, ios::end);
    auto end_pos = fin.tellg();
    fin.seekg(begin_pos);

    string content;
    content.resize(end_pos - begin_pos);
    fin.read(content.data(), static_cast<streamsize>(content.size()));
    return content;
}

string read_file_using_tellg_and_read_and_override(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    fin.exceptions(ios::failbit | ios::badbit);

    auto begin_pos = fin.tellg();
    fin.seekg(0, ios::end);
    auto end_pos = fin.tellg();
    fin.seekg(begin_pos);

    string content;
    content.resize_and_overwrite(end_pos - begin_pos,
                                 [&fin](auto* p, auto n) { return fin.read(p, n).gcount(); });

    return content;
}

string read_file_using_ostringstream(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    fin.exceptions(ios::failbit | ios::badbit);

    ostringstream oss;
    oss << fin.rdbuf();
    return std::move(oss).str();
}

string read_file_using_stream_extraction(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    fin.exceptions(ios::failbit | ios::badbit);

    ostringstream oss;
    fin >> noskipws >> oss.rdbuf();
    return std::move(oss).str();
}

string read_file_using_iterators(const filesystem::path& file_name)
{
    ifstream fin(file_name, ios::binary);
    fin.exceptions(ios::failbit | ios::badbit);

    return {istreambuf_iterator {fin}, {}};
}

void benchmark_file_read_methods(const filesystem::path& file_name, int iterations)
{
    vector<pair<string, function<string(const filesystem::path&)>>> methods = {
        {                   "file_size + fin.read", read_file_using_filesize_and_read},
        {                       "tellg + fin.read",    read_file_using_tellg_and_read},
        {"tellg + fin.read + resize_and_overwrite",    read_file_using_tellg_and_read},
        {                     "oss << fin.rdbuf()",     read_file_using_ostringstream},
        {         "fin >> noskipws >> oss.rdbuf()", read_file_using_stream_extraction},
        {                    "istreambuf_iterator",         read_file_using_iterators}
    };

    for (const auto& [name, method] : methods)
    {
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) { volatile auto content = method(file_name); }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        cout << name << " cost: " << duration.count() << " ms\n";
    }
}

int main()
{
    const filesystem::path test_file = "test_file.bin";
    const int file_size = 100 * 1024 * 1024; // 100 MB
    const int iterations = 10;

    {
        ofstream fout(test_file, ios::binary);
        vector<char> data(file_size, 'A');
        fout.write(data.data(), file_size);
    }

    cout << "test_file.size(): " << file_size / (1024 * 1024) << " MB, iter time: " << iterations
         << "\n";
    benchmark_file_read_methods(test_file, iterations);


    filesystem::remove(test_file);
}

// test_file.size(): 100 MB, iter time: 10
// file_size + fin.read cost: 396 ms
// tellg + fin.read cost: 363 ms
// tellg + fin.read + resize_and_overwrite cost: 357 ms
// oss << fin.rdbuf() cost: 660 ms
// fin >> noskipws >> oss.rdbuf() cost: 654 ms
// istreambuf_iterator cost: 2746 ms
