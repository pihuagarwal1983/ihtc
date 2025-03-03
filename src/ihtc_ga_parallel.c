#include <definition.h>
#define _POSIX_C_SOURCE 199309L
#define num_threads 4
typedef struct {
    HeapNode** data;
    int size;
    int capacity;
} Mand_opt_PQ;
extern int days;
extern int skill_levels;
extern int num_occupants;
extern int num_patients;
extern int num_surgeons;
extern int num_ots;
extern int num_rooms;
extern int num_nurses;
extern int mandatory_count;
extern int optional_count;
extern int current_ot_index;
pthread_mutex_t mutex =  PTHREAD_MUTEX_INITIALIZER;
time_t start_time;
//extern void admitPatients

extern RoomVector* v_A, * v_B, * v_empty;

extern const int POPULATION_SIZE, NUM_ITER, CONVERGENCE_STOPPING_CRITERION; int CHROMOSOME_SIZE_NEW;
extern int** POPULATION, * G_BEST, CHROMOSOME_SIZE,size;
extern int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
extern int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, * chromosome;
int** best_chromosomes;

extern Nurses* nurses;
extern Shifts* shift;
extern Weights* weights;
extern Occupants* occupants;
extern Patient* patients;
extern Patient** mandatory_patients;
extern Patient** optional_patients;
extern char**** room_schedule;
extern Surgeon* surgeon;
extern OTs* ot;
extern Rooms* room;
extern int k;
extern int* room_gender_map;

//...........FUNCTIONS............//
extern void parse_occupants(cJSON* occupants_array);
extern void free_occupants();
extern void assign_occupants_to_rooms(void);
extern void parse_weights(cJSON* weights_json);
extern void parse_patients(cJSON* patients_array);
extern void free_patients();
extern void assign_patients_to_surgeon(void);
extern void free_surgeons();
extern void parse_ots(cJSON* ot_array);
extern void free_ots();
extern void parse_rooms(cJSON* room_array);
extern void free_rooms(void);
extern void parse_nurse(cJSON* nurses_array);
extern void print_occupants();
extern void parse_json(const char* filename);
extern int compare_surgeon(const void* a, const void* b);
extern void sort_s_data_arr(Surgeon_data** s_data_arr, int n, Node* head);
extern void init_Rooms(RoomVector* vector, int initialcap);
extern void pushback(RoomVector* vector, Rooms* room);
extern Rooms* removebyid(RoomVector* vector, int room_id);
extern void moveRoom(RoomVector* source, RoomVector* destination, int room_id);
extern void freevector(RoomVector* vector);
extern void make_3_vectors(int** room_gender_map);
//extern int find_max_surgeon_id(Node* head);
extern int findSuitableRoom(int p_id, RoomVector* vector);
extern void init_Mand_opt_PQ(Mand_opt_PQ* vector, int initial_cap);
extern void resizevector(Mand_opt_PQ* vector);
extern void pushback_Mand_Opt_PQ(Mand_opt_PQ* vector, HeapNode* node);
extern HeapNode* pop_Mand_opt_PQ(Mand_opt_PQ* vector);
extern void heapifyUp(PriorityQueue* pq, int index);
extern void heapifyDown(PriorityQueue* pq, int index);
extern void initPQ(PriorityQueue* pq, int initialCapacity);
extern void insertNodeInPQ(PriorityQueue* pq, HeapNode node);
extern HeapNode extractMaxFromPQ(PriorityQueue* pq);
extern HeapNode peekMaxFromPQ(PriorityQueue* pq);
extern int isEmptyPQ(PriorityQueue* pq);
extern int compareNodesForPQ(HeapNode a, HeapNode b);
extern void resizePQ(PriorityQueue* pq);
extern HeapNode makeHeapNode(int p_id, int mandatory, int due_day, int delay, int los);
extern void initialize_room_gender_map(int** room_gender_map);
extern void populate_room_gender_map(int** room_gender_map);
extern void create_json_file(Patient* patients, int num_patients, Nurses* nurse, int num_nurses, int num_rooms, const char* instance_name, const char* output_folder);
extern void create_dm_nurses_availability();
extern void sorting_nurse_id_max_load();
extern void create_3d_array();
extern void print_room_schedule();
extern void initialize_rooms_req(num_rooms);
extern void create_rooms_req();
extern void print_rooms_req();
extern void nurse_assignments();
extern void print_rooms();
extern void free_nurses();
extern void empty_pq(PriorityQueue* pq);
extern void update_LOS_of_patients_GA(int d, int* chromosome);
extern OTs* admitFromPQ_GA(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt, int* fitness_storage_place);
extern void reset_values();
extern int compare_release_Day(const void* a, const void* b);

//...........GA FUNCTIONS............//
void applyGeneticAlgorithm3(PriorityQueue* pq);
extern int evaluateFitnessScore(int* chromosome, PriorityQueue* pq);
extern void orderCrossover(void);
extern void swapMutation(void);
extern void generateNewChromosome(int chromo_num);
extern void generatePopulation(void);
//extern void crossoverTournamentSelection(void);
extern void crossoverTournamentSelection_temp(void);
//void mutationTournamentSelection(void);
extern void mutationTournamentSelection_temp(void);
extern void crossoverElitism(void);
extern void mutationElitism(void);
extern void initDataStructures(void);
extern void freeDataStructures(void);
extern void printPopulation(void);
void initializebest_chromosomes(void);
void print_best_chromosomes(void);
void append_optionals(void);

void initializebest_chromosomes(void) {
    best_chromosomes = (int**)calloc(4, sizeof(int*));  // Allocate 4 rows (4 chromosomes)
    if (!best_chromosomes) {
        perror("Memory allocation failed for best_chromosomes");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++) {  // Only loop for 4 best chromosomes
        best_chromosomes[i] = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
        if (!best_chromosomes[i]) {
            perror("Memory allocation failed for best_chromosomes[i]");
            exit(EXIT_FAILURE);
        }
    }
}


void print_best_chromosomes(void) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < CHROMOSOME_SIZE; j++) {
			printf("%d ", best_chromosomes[i][j]);
		}
		printf("\n");
	}
}


void applyGeneticAlgorithm3(PriorityQueue* pq)
{
    int i, j, best_fitness, g_best, count = 0;
    unsigned int same_fitness_iter;
    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
    generatePopulation();
    for (i = 0; i < POPULATION_SIZE; ++i) {
        //reset_values();
        POPULATION[i][CHROMOSOME_SIZE] = evaluateFitnessScore(POPULATION[i], pq);
        bool isduplicate = false;
        best_fitness = POPULATION[i][CHROMOSOME_SIZE];
        if (best_fitness == mandatory_count) {
            for (int i = 0; i < CHROMOSOME_SIZE; i++) {
                if (POPULATION[i][CHROMOSOME_SIZE] != best_chromosomes[0][i] && POPULATION[i][CHROMOSOME_SIZE] != best_chromosomes[1][i] && POPULATION[i][CHROMOSOME_SIZE] != best_chromosomes[2][i] && POPULATION[i][CHROMOSOME_SIZE] != best_chromosomes[3][i]) {
                    isduplicate = false;
                    break;
                }
                else {
                    isduplicate = true;
                }
            }
                if (count < 4 && !isduplicate) {
                    memcpy(best_chromosomes[count], POPULATION[i], (CHROMOSOME_SIZE + 1) * sizeof(int));
                    count++;
                }
            }
        
    }
    for (int i = 0; i < CHROMOSOME_SIZE; i++) {
        G_BEST[i] = POPULATION[0][i];
    }
    g_best = G_BEST[CHROMOSOME_SIZE];
    //printPopulation();

    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    for (i = 0; i <= NUM_ITER; ++i) {
       /* if (count == 4) {
            break;
        }*/
        if ((rand() / (float)RAND_MAX) <= p_c) {
            crossoverTournamentSelection_temp();
            orderCrossover();
            // calculate the fitness of the 2 new offsprings
            for (j = 0; j < 2; ++j)
                CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE] = evaluateFitnessScore(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq);
            crossoverElitism();
            for (j = 0; j < 2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE + 1));
        }
        else {
            mutationTournamentSelection_temp();
            swapMutation();
            // calculate the fitness of the new offspring
            MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] = evaluateFitnessScore(MUTATED_OFFSPRING_STORAGE_PLACE, pq);
            mutationElitism();
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 1));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        bool isduplicate = false;
        best_fitness = G_BEST[CHROMOSOME_SIZE];
        /*if (best_fitness == mandatory_count) {
            for (int i = 0; i < CHROMOSOME_SIZE; i++) {
                if (G_BEST[i] != best_chromosomes[0][i] && G_BEST[i] != best_chromosomes[1][i] && G_BEST[i] != best_chromosomes[2][i] && G_BEST[i] != best_chromosomes[3][i]) {
                    isduplicate = false;
                    break;
                }
                else {
                    isduplicate = true;
                }
                if (!isduplicate) {
                    memcpy(best_chromosomes[count], G_BEST, (CHROMOSOME_SIZE + 1) * sizeof(int));
                    count++;
                }
            }
        }*/


    }
    printf("Number of iterations: %d", i);
}



void append_optionals(void) {
    int* optionals = (int*)calloc(optional_count, sizeof(int));
    if (!optionals) {
        perror("Memory allocation failed for optionals");
        exit(EXIT_FAILURE);
    }

    // Store optional patient IDs in the array
    for (int i = 0; i < optional_count; i++) {
        optionals[i] = optional_patients[i]->id;
    }

    // Append to best_chromosomes
    for (int i = 0; i < 4; i++) {
        int new_size = CHROMOSOME_SIZE + optional_count + 1;
        int* temp = (int*)realloc(best_chromosomes[i], new_size * sizeof(int));
        if (!temp) {
            perror("Memory allocation failed for best_chromosomes[i]");
            exit(EXIT_FAILURE);
        }
        best_chromosomes[i] = temp;

        // Append optionals after the chromosome
        for (int j = 0; j < optional_count; j++) {
            best_chromosomes[i][CHROMOSOME_SIZE  + j] = optionals[j];
        }
		best_chromosomes[i][new_size - 1] = evaluateFitnessScore(best_chromosomes[i], NULL);
        // Optional: Print to check
        printf("Best Chromosome %d after appending: ", i);
        for (int k = 0; k < new_size; k++) {
            printf("%d ", best_chromosomes[i][k]);
        }
        printf("\n");
    }

    free(optionals);
}


void admitPatientsParallel(int** room_gender_map,  int* chromosome) {
    int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, day, p_counter;
    int unscheduled_mandatory = 0, scheduled_optional_count = 0, assigned_ot, flag, max = 0;
    OTs** ot_data_arr, * current_ot;

    v_A = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_B = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_empty = (RoomVector*)calloc(1, sizeof(RoomVector));
    //populate_room_gender_map(&room_gender_map);

    make_3_vectors(&room_gender_map);
    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    if (ot_data_arr == NULL) {
        printf("Memory not allocated.\n");
        exit(-1);
    }
    //print_ots(ot);
    for (i = 0; i < num_ots; ++i)
        ot_data_arr[i] = ot + i;

    // printf("\nChromosome:%d\t", k);
    for (int i = 0; i < CHROMOSOME_SIZE + CHROMOSOME_SIZE_NEW; i++)
        if (max < chromosome[i])
            max = chromosome[i];

    int* unscheduled_mandatory_patients = (int*)calloc(max + 1, sizeof(int));
    int* scheduled_optional_patients = (int*)calloc(max + 1, sizeof(int));
    for (day = 0; day < days; day++) {
        sort_ot_data_arr(ot_data_arr, day);
        if (day) {
            update_LOS_of_patients_GA(day, chromosome);
            //if (pq->current_size)
              //  current_ot = admitFromPQ_GA(pq, day, ot_data_arr, current_ot, current_ot_index, 0, chromosome);
        }
        current_ot_index = 0;
        current_ot = ot_data_arr[current_ot_index];
        for (p_counter = 0; p_counter < CHROMOSOME_SIZE; p_counter++) {
            p_id = chromosome[p_counter];

            if (patients[p_id].surgery_release_day > day || patients[p_id].admission_day != -1) {
                continue;
            }
            else {
                if ((patients[p_id].mandatory && patients[p_id].surgery_due_day < day && patients[p_id].admission_day == -1) && unscheduled_mandatory_patients[p_id] == 0) {
                    ++unscheduled_mandatory;
                    unscheduled_mandatory_patients[p_id] = 1;
                    continue;
                }
                else {
                    if (patients[p_id].surgery_release_day <= day) {
                        if (patients[p_id].mandatory && patients[p_id].surgery_due_day < day) continue;
                        s_id = patients[p_id].surgeon_id;
                        // check if the surgeon is available
                        if (surgeon[s_id].time_left[day] < patients[p_id].surgery_duration) {
                            //if (patients[p_id].mandatory && patients[p_id].surgery_release_day == day)
                              //  insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                            continue;
                        }
                        // check if the OT is available
                        if (current_ot->time_left[day] < patients[p_id].surgery_duration) {
                            for (j = 0; j < num_ots || assigned_ot == -1; j++) {
                                if (ot_data_arr[j]->time_left[day] >= patients[p_id].surgery_duration) {
                                    assigned_ot = ot_data_arr[j]->id;
                                    break;
                                }
                            }
                            if (assigned_ot != -1) {
                                if (j < num_ots) {
                                    current_ot_index = j;
                                    current_ot = ot_data_arr[current_ot_index];
                                    assigned_ot = current_ot->id;
                                }
                                else {
                                    //if (patients[p_id].mandatory && patients[p_id].surgery_release_day == day)
                                      //  insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                                    continue;
                                }
                                //ot_data_arr[current_ot_index]->time_left[day] -= patients[p_id].surgery_duration;
                            }
                            else {
                                // if true - it means no ot could be assigned to this patient - put him in PQ
                                //if (patients[p_id].mandatory && patients[p_id].surgery_release_day == day)
                                  //  insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                                continue;
                            }
                        }
                        else {
                            assigned_ot = current_ot->id;
                        }

                        // if the control comes here - that means surgeon and OT are available and only room remains
                        // look for a suitable roo
                        r_id = (patients[p_id].gen) ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                        if (r_id != -1 && surgeon[patients[p_id].surgeon_id].time_left[day] >= patients[p_id].surgery_duration) {
                            room[r_id].num_patients_allocated++;
                            patients[p_id].admission_day = day;
                            patients[p_id].assigned_ot = assigned_ot;
                            patients[p_id].assigned_room_no = r_id;
                            patients[p_id].is_admitted = 1;
                            surgeon[patients[p_id].surgeon_id].time_left[day] -= patients[p_id].surgery_duration;
                            ot[assigned_ot].time_left[day] -= patients[p_id].surgery_duration;
                            if (patients[p_id].mandatory == 0) {
                                scheduled_optional_patients[p_id] = 1;
                                scheduled_optional_count++;
                            }

                        }
                        else {
                            //if (patients[p_id].mandatory && patients[p_id].surgery_release_day == day)
                              //  insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                            continue;
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < max + 1; i++) {
        if (patients[i].admission_day == -1 && patients[i].mandatory) {
            unscheduled_mandatory++;
            unscheduled_mandatory_patients[i] = 1;
        }
    }

    for (int i = 0; i < max + 1; i++) {
        if (patients[i].admission_day != -1 && !patients[i].mandatory) {
            scheduled_optional_count++;
            scheduled_optional_patients[i] = 1;
        }
    }

    // put the fitness values in the right place
    chromosome[CHROMOSOME_SIZE] = mandatory_count - unscheduled_mandatory;
    chromosome[CHROMOSOME_SIZE + 1] = scheduled_optional_count + mandatory_count - unscheduled_mandatory;
    free(unscheduled_mandatory_patients);
    free(scheduled_optional_patients);
    free(v_A);
    free(v_B);
    free(v_empty);

}


#include <windows.h>

double get_time_in_seconds() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / freq.QuadPart;
}


void swapMutationParallel(int* chromosome) {
    int start = CHROMOSOME_SIZE;  // Corrected start index
    int end = CHROMOSOME_SIZE + CHROMOSOME_SIZE_NEW - 1;  // Corrected end index

    if (start >= end) return;  // Prevent invalid range

    int idx1 = start + rand() % (end - start + 1);
    int idx2 = start + rand() % (end - start + 1);

    while (idx1 == idx2) {  // Ensure different indices
        idx2 = start + rand() % (end - start + 1);
    }

    // Swap two genes
    int temp = chromosome[idx1];
    chromosome[idx1] = chromosome[idx2];
    chromosome[idx2] = temp;
}



#define MUTATION_PROBABILITY 0.2

void printChromosome(int* chromosome, int size, int thread_index) {
    printf("Thread %d - Final Chromosome: [", thread_index);
    for (int i = 0; i < size; i++) {
        printf("%d", chromosome[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\n");
}

void* mutateChromosome(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int index = args->index;
    int* chromosome = args->chromosome;

    LARGE_INTEGER frequency, start_time, current_time;
    QueryPerformanceFrequency(&frequency);  // Get high-resolution timer frequency
    QueryPerformanceCounter(&start_time);   // Get start time

    int elapsed_minutes = 0;
    while (1) {
        QueryPerformanceCounter(&current_time);  // Get current time
        double elapsed_time = (double)(current_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
       // printf("Thread %d started execution\n", index);
        // Check if 10 minutes have passed
        if (elapsed_time >= 600) {
            break;
        }

        pthread_mutex_lock(&mutex);
        if ((double)rand() / RAND_MAX < MUTATION_PROBABILITY) {
            swapMutationParallel(chromosome);
        }
        pthread_mutex_unlock(&mutex);

        // Check if 2 minutes have passed
        if ((int)elapsed_time / 120 > elapsed_minutes) {
            elapsed_minutes++;

           // printf("Thread %d pausing to admit patients...\n", index);
            pthread_mutex_lock(&mutex);
            reset_values();
            admitPatientsParallel(&room_gender_map, chromosome);  // Call admit patients
            pthread_mutex_unlock(&mutex);
        }

        Sleep(50000);  // Sleep for 50ms to prevent excessive CPU usage
    }


    printf("Thread %d finished after 10 minutes.\n", index);

    pthread_mutex_lock(&mutex);
    //printChromosome(chromosome, CHROMOSOME_SIZE + CHROMOSOME_SIZE_NEW, index);
    pthread_mutex_unlock(&mutex);

    free(args);
    pthread_exit(NULL);
}

//int patient_delay() {
//    int cost = 0;
//    for (int p = 0; p < num_patients; p++) {
//        if (patients[p].admission_day != -1 && patients[p].admission_day > patients[p].surgery_release_day) {
//            cost += patients[p].admission_day - patients[p].surgery_release_day;
//        }
//    }
//
//    return cost;
//}
//
//int elective_unscheduled_patients() {
//    int cost = 0;
//    for (int i = 0; i < num_patients; i++) {
//        if (patients[i].admission_day == -1 && !patients[i].mandatory) {
//            cost++;
//        }
//    }
//    return cost;
//}
//
//int open_operating_theatre() {
//    int cost = 0;
//    for (int t = 0; t < num_ots; t++) {
//        for (int d = 0; d < days; d++) {
//            if (ot[t].max_ot_time[d] != ot[t].time_left[d]) {
//                cost++;
//            }
//        }
//    }
//    return cost;
//}


//int room_age_mix() {
//    int cost = 0, min1 = 0, min2 = 0, max1 = 0, max2 = 0, max, min;
//    for (int i = 0; i < num_rooms; i++) {
//        for (int j = 0; j < days; j++) {
//            if (size_of_room_schedule[i][j]) {
//                min1 = room_schedule[i][j][0];
//                max1 = min1;
//                for (int m = 0; m < size_of_room_schedule[i][j]; m++) {
//                    int g = room_schedule[i][j][m]->gen;
//                    if (g < min1)
//                        min1 = g;
//                    else if (g > max1)
//                        max1 = g;
//                }
//                if (room[i].occupants_cap) {
//                    for (int k = 0; k < room[i].occupants_cap; k++) {
//                        int g = occupants[room[i].occ[k]].gen;
//                        if (g < min2)
//                            min2 = g;
//                        else if (g > max2)
//                            max2 = g;
//                    }
//                }
//            }
//            if (min1 < min2) min = min1;
//            else min = min2;
//
//            if (max1 < max2) max = max2;
//            else max = max1;
//
//            cost += max - min;
//        }
//    }
//
//    return cost;
//
//}


//int main() { 
//    srand(time(NULL));
//    start_time = time(NULL);
//    parse_json("data/instances/i08.json");
//    PriorityQueue* pq;
//    CHROMOSOME_SIZE = mandatory_count;
//    CHROMOSOME_SIZE_NEW = optional_count;
//    srand(0);
//    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
//    if (!pq)
//        ASSERT(0, "Dynamic Memory Allocation Error for PQ");
//    initDataStructures();
//    initialize_room_gender_map(&room_gender_map);
//    initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
//    populate_room_gender_map(&room_gender_map);
//    print_map(&room_gender_map);
//    generatePopulation();
//    //reset_values();
//	initializebest_chromosomes();
//	applyGeneticAlgorithm3(pq);
// 
//	printf("Best Chromosomes: \n");
//	print_best_chromosomes();
//	append_optionals();
//	//printf("Best Chromosomes after appending optionals: \n");
//	//print_best_chromosomes();
//    pthread_t threads[num_threads];
//    pthread_mutex_init(&mutex, NULL);
//
//    // Create threads
//    for (int i = 0; i < num_threads; i++) {
//        ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
//        args->index = i;
//        args->chromosome = best_chromosomes[i];
//        //printf("Creating thread %d\n", i);
//        pthread_create(&threads[i], NULL, mutateChromosome, args);
//       // printf("Thread %d created successfully\n", i);
//
//    }
//
//    // Wait for all threads to complete
//    for (int i = 0; i < num_threads; i++) {
//        pthread_join(threads[i], NULL);
//    }
//
//    create_json_file(patients, num_patients, nurses, num_nurses, num_rooms, "i08", "D:/major_code/build/output");
//
//	freeDataStructures();
//	free(pq);
//	return 0;
//}