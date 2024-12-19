#include <iostream>
#include <vector>
#include <string>
#include <map>

int main() {
    std::vector<std::string> testStrings {"this", "is", "just", "a", "test"};
    std::string my_var = "Hello";
    int my_int = 132;
    double my_dbl = 12.43;
    std::map <std::string, int> my_map;
    my_map["one"] = 1;
    std::vector<int> vec_int = {1,2,3,4};

    for(auto testString : testStrings) {
        std::cout << testString << std::endl;
    }
}