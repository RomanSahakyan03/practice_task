#include <pthread.h>
#include <string>
#include <iostream>
#include <algorithm>

int ProccessUserInput(){
    while(true){

        std::string input;

        // Inputing a string of digits (not longer than 64 characters)
        std::getline(std::cin, input);

        // Check that the input string only consists of digits
        if (!std::all_of(input.begin(), input.end(), ::isdigit)) {
            std::cout << "Invalid input: the string must only consist of digits" << std::endl;
            continue;
        }

        // Check that the input string does not exceed 64 characters
        if (input.length() > 64) {
        std::cout << "Invalid input: the string must not be longer than 64 characters" << std::endl;
        continue;
        }

        // Sort the input string in descending order
        std::sort(input.rbegin(), input.rend());

        // Replace even elements with "KB"
        for(auto it = input.begin(); it != input.end(); ++it){
            if((*it - '0') % 2 == 0){
                *it = 'K';
                input.insert(++it, 'B');
            }
        }
    }
}



int main(int argc, char **argv){

    return 0;

}