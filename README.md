# INIT Parser in C++

This is a collection of code for parsing INIT files in C++.

See the document `Init file format specification.pdf` for information on the file type

## Usage

Given a file of INIT format named `test.init`

This library can be used as follows

```c++
int main() {
    auto f = Init::InitFile::parse("test.init");
    
    auto value = f.sections().getRecursive("some_key");
    
    if (value.has_value()) {
        std::cout << *value << std::endl;
    }
    
    auto section_path = f.getPathToEntry("some_other_key");
    
    f.updateEntryRecursive(section_path, "some_other_key", "another_value");
    
    std::ofstream out{};
    out.open("new_test.init");
    f.print(out);
    out.close()
    
    // ... etc.
    return 0;
}

```

Most methods return an optional if the key is present. Most methods also come in regular and recursive forms.
The regular forms (such as `hasEntry()`, `getEntry()`, and `updateEntry()`) operate only on the section on which they
are called. While the
recursive versions will traverse starting at the section they are called on trying to find the key in subsections

You can use the `getPathToEntry()` method to determine the subsection path that contains a key. This method returns:

- An optional of a `std::vector` of nested subsection names (`std::string`) that lead to the section which contains the
  key if the key is contained in some subsection
- A **present** optional of an **empty vector** if the key is contained in the default section (no section name is
  present or required to retrieve it)
- `std::nullopt` if the key is not found anywhere in the hierarchy