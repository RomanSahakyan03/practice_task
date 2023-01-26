#include <pthread.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h> 

pthread_cond_t  cv;
pthread_mutex_t mtx;

void ProccessUserInput(){
    while(true){

        pthread_mutex_lock(&mtx);
        pthread_cond_wait(&cv, &mtx);

        std::string input;

        std::cout << "Enter a string of digits (not longer than 64 characters)" << std::endl;
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

        // Put the processed input string in the shared buffer
        *shared_buffer = input;

        // Wait for further user input
        // ...
    }
}



int main(int argc, char **argv) {

    // Create a shared memory object
    int shm_fd = shm_open("/my_shared_buffer", O_CREAT | O_RDWR, 0644);



    // Set the size of the shared memory object
    ftruncate(shm_fd, 96);

    // Map the shared memory object to the virtual memory of the process
    std::string* shared_buffer = (std::string*) mmap(NULL, sizeof(std::string), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


    // Use the shared buffer in your threads
    // ...

    // Unmap the shared memory object
    //munmap(shared_buffer, BUFFER_SIZE);

    shm_unlink("/my_shared_buffer");
    return 0;
}