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

        // Lock the shared buffer to prevent race conditions
        pthread_mutex_lock(&mtx);

        while(!shared_buffer->empty())
        {
            // Wait for a signal indicating that the shared buffer is free to write to
            pthread_cond_wait(&cv, &mtx);
        }

        std::string input;

        std::cout << "Enter a string of digits (not longer than 64 characters)" << std::endl;
        std::getline(std::cin, input);

        // Check that the input string only consists of digits
        if (!std::all_of(input.begin(), input.end(), ::isdigit)) {
            std::cout << "Invalid input: the string must only consist of digits" << std::endl;
            // Release the lock and continue to the next iteration of the loop
            pthread_mutex_unlock(&mtx);
            continue;
        }

        // Check that the input string does not exceed 64 characters
        if (input.length() > 64) {
        std::cout << "Invalid input: the string must not be longer than 64 characters" << std::endl;
        // Release the lock and continue to the next iteration of the loop
        pthread_mutex_unlock(&mtx);
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

        // Signal that the buffer has been written to
        pthread_cond_signal(&cv);

        // Release the lock
        pthread_mutex_unlock(&mtx);
    }
}

void* PrintProcessedInput(void* args) {
    // Cast the argument to a pointer to a string object
    std::string *shared_buffer = static_cast<std::string *>(args);
    while(true) {
        // Lock the mutex before accessing the shared buffer
        pthread_mutex_lock(&mtx);
        while(shared_buffer->empty()) {
            // Wait on the condition variable if the buffer is empty
            pthread_cond_wait(&cv, &mtx);
        }

        // Print the processed input string
        std::cout << "Processed input: " << *shared_buffer << std::endl;

        // Clear the buffer after reading
        shared_buffer->clear();

        // Signal that the buffer has been read
        pthread_cond_signal(&cv);

        // Unlock the mutex after accessing the shared buffer
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}


int main(int argc, char **argv) {

    // Create a shared memory object
     int shm_id = shmget(IPC_PRIVATE, 1024, 0644 | IPC_CREAT);

    // Check if the shared memory was created successfully
     if (shm_id == -1) {
        // Print an error message if the shared memory was not created
        std::cerr << "Error creating shared memory." << std::endl;
        return 1;
    }


    // Attach the shared memory to the process's address space
    void *shm_ptr = shmat(shm_id, NULL, 0);


    if (shm_ptr == (void*)-1) {
        std::cerr << "Error attaching shared memory." << std::endl;
        return 1;
    }


    // Create two threads
    pthread_t thread1, thread2;

    // Pass the shared buffer pointer to the thread functions
    pthread_create(&thread1, NULL, ProccessUserInput, (void*) shm_ptr);
    pthread_create(&thread2, NULL, PrintProcessedInput, (void*) shm_ptr);
    
    // Join threads
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Detach the shared memory from the process's address space
    shmdt(shm_ptr);

    // Mark the shared memory for destruction
    shmctl(shm_id, IPC_RMID, NULL);
    
    return 0;
}