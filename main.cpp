#include <pthread.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h> 
#include <unistd.h>
#include <sys/shm.h>

pthread_cond_t  cv;
pthread_mutex_t mtx;

void *ProccessUserInput(void *args){
    std::string *shared_buffer = static_cast<std::string *>(args);
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

void* PrintProcessedInput(void* args) {
    std::string *shared_buffer = static_cast<std::string *>(args);
    while(true) {
        pthread_mutex_lock(&mtx);
        while(shared_buffer->empty()) {
            pthread_cond_wait(&cv, &mtx);
        }

        // Print the processed input string
        std::cout << "Processed input: " << *shared_buffer << std::endl;

        shared_buffer->clear();

        // Signal that the buffer has been read
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}


int main(int argc, char **argv) {

    // Create a shared memory object
     int shm_id = shmget(IPC_PRIVATE, 1024, 0644 | IPC_CREAT);

     if (shm_id == -1) {
        std::cerr << "Error creating shared memory." << std::endl;
        return 1;
    }


    void *shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        std::cerr << "Error attaching shared memory." << std::endl;
        return 1;
    }


    pthread_t thread1, thread2;

    // Use the shared buffer in your threads
    pthread_create(&thread1, NULL, ProccessUserInput, (void*) shm_ptr);
    pthread_create(&thread2, NULL, PrintProcessedInput, (void*) shm_ptr);
    
    
    return 0;
}