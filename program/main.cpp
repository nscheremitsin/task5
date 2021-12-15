#include <cstdio>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cmath>
#include <pthread.h>

int EXPECTED_ARGS_AMOUNT = 3;

pthread_mutex_t mutex;

int regions_amount;
int groups_amount;
int treasures_amount;

bool* regions;
int* result_treasured_regions_indexes;
int regions_amount_for_each_thread;
int treasures_count = 0;

// Prints message about expected command line format.
void printExpectedFormatError() {
    printf(
        "Error! Expected command format:\n"
        "   <target> <R> <G> <T>\n"
        "where:\n"
        "   target — built file that runs program\n"
        "   R — amount of regions   — integer in range [1, %d]\n"
        "   G — amount of groups    — integer in range [1, R]\n"
        "   T — amount of treasures — integer in range [1, R]",
        INT_MAX
   );
}

// Checks entered parameters validity.
int* readCommandLineArgs(int argc, char* argv[]) {
    if (argc != EXPECTED_ARGS_AMOUNT + 1) {
        return nullptr;
    }
    int* args = new int[EXPECTED_ARGS_AMOUNT];

    for (int i = 0; i < EXPECTED_ARGS_AMOUNT; ++i) {
        int arg = atoi(argv[i + 1]);

        if (arg <= 0 || i != 0 && arg > args[0]) {
            return nullptr;
        }
        args[i] = arg;
    }
    // Warning message.
    if (log10(args[0]) + log10(args[2]) > 9) {
        puts("Please, notice that because your R + T > 10^9, the program may run for too long!\n");
    }
    return args;
}

// Randomly populates regions array with treasures.
void placeTreasuresInRegions() {
    // Array contains remaining regions to be randomly chosen for placing treasure.
    int* remaining_regions_indexes = new int[regions_amount];
    int remaining_regions_amount = regions_amount;

    // Initialization.
    for (int i = 0; i < regions_amount; ++i) {
        regions[i] = false;
        remaining_regions_indexes[i] = i;
    }

    srand(time(nullptr));
    for (int i = 0; i < treasures_amount; ++i) {

        // Random region index.
        int index = rand() % remaining_regions_amount;
        remaining_regions_amount--;
        // Placing treasure.
        regions[remaining_regions_indexes[index]] = true;

        // Shifting array to make sure that chosen index will not be chosen again.
        for (int j = index; j < remaining_regions_amount; ++j) {
            remaining_regions_indexes[j] = remaining_regions_indexes[j + 1];
        }
    }
    delete[] remaining_regions_indexes;
}

void* searchRegions(void* param) {
    int thread_number = *(int*)param;
    // Initialized by start index for current thread.
    int index = regions_amount_for_each_thread * thread_number;

    for (int i = 0; i < regions_amount_for_each_thread; ++i) {
        // Condition can be reached by last thread due to regions_amount_for_each_thread calculation.
        if (index >= regions_amount) {
            break;
        }
        bool is_treasure_found = regions[index];

        // Critical section opens.

        pthread_mutex_lock(&mutex);
        if (is_treasure_found) {
            // Remembering region with treasure.
            result_treasured_regions_indexes[treasures_count] = index + 1;
            treasures_count++;
        }
        pthread_mutex_unlock(&mutex);

        // Critical section is closed.

        // Message about search results.
        if (is_treasure_found) {
            printf("Group %d FOUND treasure in region %d\n", thread_number + 1, index + 1);
        } else {
            printf("Group %d did not found anything in region %d\n", thread_number + 1, index + 1);
        }
        index++;
    }
    return nullptr;
}

void findRegionsWithTreasures() {
    // Threads storage.
    pthread_t* threads = new pthread_t[groups_amount];
    // Array contains threads' numbers to be passed in function.
    int* threads_numbers = new int [groups_amount];

    // Starting threads.
    for (int i = 0; i < groups_amount; ++i) {
        pthread_t thread;
        threads_numbers[i] = i;
        pthread_create(&thread, nullptr, searchRegions, (void*)&threads_numbers[i]);
        threads[i] = thread;
    }
    // Joining threads.
    for (int i = 0; i < groups_amount; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete[] threads;
    delete[] threads_numbers;
}

int main(int argc, char* argv[]) {
    // Getting integer args from received params.
    int* args = readCommandLineArgs(argc, argv);
    if (!args) {
        printExpectedFormatError();
        return EXIT_FAILURE;
    }
    regions_amount = args[0];
    groups_amount = args[1];
    treasures_amount = args[2];
    delete[] args;

    // + 1 in order to include remaining regions (regions_amount % groups_amount).
    regions_amount_for_each_thread = regions_amount / groups_amount + 1;
    pthread_mutex_init(&mutex, nullptr);

    regions = new bool[regions_amount];
    result_treasured_regions_indexes = new int[treasures_amount];

    // Preparing.
    placeTreasuresInRegions();
    // Calculating.
    findRegionsWithTreasures();

    // Printing search results.
    printf("\nTreasures have been found in following regions:\n");
    for (int i = 0; i < treasures_amount; ++i) {
        printf("%d ", result_treasured_regions_indexes[i]);
    }
    delete[] regions;
    delete[] result_treasured_regions_indexes;
    return 0;
}