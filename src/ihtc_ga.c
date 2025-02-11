#include <stdio.h>
//#include <ncurses.h>
//#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "cJSON.h"
#include <direct.h>    
#include <stdbool.h>


//#include <cassert>

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s\nFile: %s, Line: %d\n", message, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

// Extern declarations (telling the compiler these are defined elsewhere)
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

// int is_admitted[1000] = {0}; // we're not using this array anywhere so let's comment it out
int num_gender_A_rooms, num_gender_B_rooms, num_empty_rooms;
int* current_size_dm_nurse;
int* room_gender_map;

int **size_of_room_schedule;

// Enums for the mapped values
typedef enum { A, B } gender;
typedef enum { early, late, night } shift_types;
typedef enum { optional, mandatory } PatientType;

const char* gender_to_string(gender g);

const char* shift_types_to_string(shift_types shift);

// Enum conversion functions
gender string_to_gender(const char* str);

shift_types string_to_shift_types(const char* str);

int str2int(char* a);

// Struct definitions
typedef struct {
   int day;
   shift_types shift_time;
   int max_load;
   int* rooms;
   int num_rooms;
} Shifts;

typedef struct {
   int id;
   int skill_level;
   Shifts* shift;
   int num_shifts;
} Nurses;

//Nurses* nurses;
Nurses*** dm_nurses_availability;  //nurse assignment
int** max_load_updated; //nurse assignment

typedef struct {
   int room_mixed_age;
   int room_nurse_skill;
   int continuity_of_care;
   int nurse_excessive_workload;
   int open_operating_theater;
   int surgeon_transfer;
   int patient_delay;
   int unscheduled_optional;
} Weights;


//Weights* weights = NULL; // Global pointer for weights
typedef struct {
   int id;
   int length_of_stay;
   int room_id;
   char* age;
   gender gen;
   int* workload_produced;
   int* skill_level_required;
} Occupants;


//Occupants* occupants = NULL; // Global pointer for occupants
typedef struct {
   int id;
   int mandatory;
   gender gen;
   char* age_group;
   int length_of_stay;
   int surgery_release_day;
   int surgery_due_day;
   int surgery_duration;
   int surgeon_id;
   int* incompatible_room_ids;
   int num_incompatible_rooms;
   int* workload_produced;
   int* skill_level_required;
   int assigned_room_no;
   int assigned_ot;
   int admission_day;
} Patient;


//Patient* patients;
//Patient** mandatory_patients = NULL;
//Patient** optional_patients = NULL;
//Patient**** room_schedule;

typedef struct {
   int id;
   int* max_surgery_time;
   int* time_left; // default: max_surgery_time
   int* patients_assigned;
   int num_assigned_patients; // a fixed number - how many patients are assigned to this surgeon
   int current_size; // dynamic size of the patients_assigned array
} Surgeon;


//Surgeon* surgeon;
typedef struct {
   int id;
   int* max_ot_time;
   int* time_left; // default: max_ot_time
} OTs;


//OTs* ot;
typedef struct {
   int id;
   int cap;
   int num_patients_allocated;
   // int *patients_allocated;
   int occupants_cap;
   int* nurses_alloted;
   int length_of_nurses_alloted;
} Rooms;

typedef struct {
    int surgeon_id;
    int* assigned_patients;
    int num_assigned_patients;
    int surgery_duration_sum;
    int isNull;
    /* added isNull in the struct cuz s_data_arr has null values as well and we need to sort that array on the basis of
    surgery_duration_sum. if that cell is null then we cant really sort the array as that may contrive problems while selecting the pivot
    so for those null pointers we'll create this same structure and put 1 in the isNull field. this way we'll know that this pointer is
    a null one and we can deal with it accordingly.
    */
} Surgeon_data;


typedef struct {
    int patient_id;  // Unique ID for patient
    int mandatory;   // 1 if mandatory, 0 otherwise
    int due_day;     // The day when the patient is due
    int delay;      // Lower is better
    int los;
} HeapNode;

typedef struct {
    HeapNode* data; // Dynamically allocated array for heap nodes
    int current_size;       // Current number of elements in the priority queue
    int capacity;   // Maximum capacity of the priority queue
} PriorityQueue;


typedef struct LL {
    Patient* pointer;
    struct LL* next, * prev;
} Node;

typedef struct {
    HeapNode** data;
    int size;
    int capacity;
} Mand_opt_PQ;

typedef struct {
    Rooms** data;
    int size;
    int capacity;
}RoomVector;

RoomVector* v_A, * v_B, * v_empty;


const int POPULATION_SIZE = 100, NUM_ITER = 10000, CONVERGENCE_STOPPING_CRITERION = 100;
int** POPULATION, * G_BEST, CHROMOSOME_SIZE;
int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, * chromosome;

extern Nurses* nurses;
extern Weights* weights;
extern Occupants* occupants;
extern Patient* patients;
extern Patient** mandatory_patients;
extern Patient** optional_patients;
extern Patient**** room_schedule;
extern Surgeon* surgeon;
extern OTs* ot;
extern Rooms* room;
// one idea is that make the patients_assigned array a static array and remove num_patients_assigned field from the structure.



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
extern void make_3_vectors(int* room_gender_map);
extern int find_max_surgeon_id(Node* head);
extern int findSuitableRoom(int p_id, RoomVector* vector);
extern void init_Mand_opt_PQ(Mand_opt_PQ* vector, int initial_cap);
extern void resizevector(Mand_opt_PQ* vector);
extern void pushback_Mand_Opt_PQ(Mand_opt_PQ* vector, HeapNode* node);
extern HeapNode* pop_Mand_opt_PQ(Mand_opt_PQ* vector);
extern void create_json_file(Patient* patients, int num_patients, Nurses* nurse, int num_nurses, int num_rooms, const char* instance_name, const char* output_folder);





OTs* admitFromPQ(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt) {
    HeapNode* node;
    int i, j, p_id, flag, r_id, s_duration, p_counter;

    Mand_opt_PQ* vector = (Mand_opt_PQ*)calloc(1, sizeof(Mand_opt_PQ));
    init_Mand_opt_PQ(vector, 20);

    /*
    if (flag_opt) {
        for (i = 0; pq->current_size > 0; i++) {
            node = (HeapNode *) calloc (1, sizeof(HeapNode));  // Allocate new node
            *node = extractMaxFromPQ(pq);  // Copy extracted node data
            pushback_Mand_Opt_PQ(vector, node);
        }
    }
    */

    for (p_counter = 0, flag = 0; p_counter < pq->current_size; ++p_counter) {
        if (!flag_opt && !(pq->data[p_counter].mandatory)) continue;
        else {
            node = (HeapNode*)calloc(1, sizeof(HeapNode));  // Allocate new node
            node = extractMaxFromPQ(pq);  // Copy extracted node data
            p_id = node->patient_id;
            s_duration = patients[p_id].surgery_duration;

            while (!current_ot->time_left[d] || current_ot->time_left[d] < s_duration) {
                current_ot = ot_data_arr[current_ot_index];
                if (current_ot_index < num_ots)
                    current_ot_index++;
                if (current_ot_index == num_ots) {
                    flag = 1;
                    current_ot_index--;
                    break;
                }
            }

            if (flag) {
                pushback_Mand_Opt_PQ(vector, node);
                continue;
            }
            else {
                r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                if (r_id == -1) {
                    pushback_Mand_Opt_PQ(vector, node);
                    continue;
                }
                else {
                    if (surgeon[patients[p_id].surgeon_id].time_left[d] >= patients[p_id].surgery_duration) {
                        patients[p_id].assigned_room_no = r_id;
                        patients[p_id].assigned_ot = current_ot->id;
                        room[r_id].num_patients_allocated++;
                        current_ot->time_left[d] -= s_duration;
                        patients[p_id].admission_day = d;
                    }
                    else
                        pushback_Mand_Opt_PQ(vector, node);
                }
            }
        }
    }

    for (int i = 0; i < vector->size; i++) {
        insertNodeInPQ(pq, *(vector->data[i]));
        free(vector->data[i]);  // Free dynamically allocated HeapNode
    }
    free(vector->data);  // Free vector data array
    free(vector);  // Free vector

    return current_ot;
}

// --------------------ABOVE: Functions for admitting optional patients from the PQ (linked list functions as well)---------------------

//-------------------------------------------------BELOW: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

int admit_patients(int* room_gender_map, PriorityQueue* pq, int* chromosome)
{
    int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, day, p_counter;
    int unscheduled_mandatory = 0, assigned_ot, flag;
    OTs** ot_data_arr, * current_ot;

    v_A = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_B = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_empty = (RoomVector*)calloc(1, sizeof(RoomVector));

    make_3_vectors(room_gender_map);
   /* printVector("A", v_A);
    printVector("B", v_B);
    printVector("Empty", v_empty);*/
        
    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    if (ot_data_arr == NULL) {
        printf("Memory not allocated.\n");
        exit(-1);
    }
    print_ots(ot);
    for (i = 0; i < num_ots; ++i)
        ot_data_arr[i] = ot + i;

    //----------------------------------------------------------apply checks and ADMIT PATIENTS------------------------------------------------------

    for (day = 0, p_counter = 0; day < days; ++day) {
        current_ot_index = 0;
        sort_ot_data_arr(ot_data_arr, day);
        current_ot = ot_data_arr[current_ot_index];

        // try to admit the patients in PQ first
        current_ot = admitFromPQ(pq, day, ot_data_arr, current_ot, current_ot_index, 0);
        if (!current_ot) {
            continue;
        }

        for (; p_counter < CHROMOSOME_SIZE; ++p_counter) {
            assigned_ot = -1;
            p_id = chromosome[p_counter];
            // if the release day is greater than current_day (day) - go to the next day and then try to admit this patient
            if (day < mandatory_patients[p_id]->surgery_release_day) {
                ++p_counter;
                break;
            }
            // if current_day (day) is greater than the due day of the patient - increase the unscheduled_mandatory count and go to the next patient
            if (day > mandatory_patients[p_id]->surgery_due_day) {
                ++unscheduled_mandatory;
                continue;
            }
            if (mandatory_patients[p_id]->surgery_release_day <= day && day <= mandatory_patients[p_id]->surgery_due_day) {
                s_id = mandatory_patients[p_id]->surgeon_id;
                // check if the surgeon is available
                if (surgeon[s_id].time_left[day] < mandatory_patients[p_id]->surgery_duration) {
                    insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id]->surgery_due_day, 0));
                    continue;
                }
                // check if the OT is available
                if (current_ot->time_left[day] < mandatory_patients[p_id]->surgery_duration) {
                    for (j = 0; j < num_ots && assigned_ot == -1; ++j) {
                        if (ot_data_arr[j]->time_left[day] >= mandatory_patients[p_id]->surgery_duration)
                            assigned_ot = ot_data_arr[j]->id;
                    }
                    if (assigned_ot != -1) {
                        current_ot_index = j;
                        current_ot = ot_data_arr[current_ot_index];
                        ot_data_arr[current_ot_index]->time_left -= mandatory_patients[p_id]->surgery_duration;
                    }
                    else {
                        // if true - it means no ot could be assigned to this patient - put him in PQ
                        insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id]->surgery_due_day, 0));
                        continue;
                    }
                }
                else {
                    assigned_ot = current_ot->id;
                    ot_data_arr[current_ot_index]->time_left -= mandatory_patients[p_id]->surgery_duration;
                }

                // if the control comes here - that means surgeon and OT are available and only room remains
                // look for a suitable room
                r_id = (mandatory_patients[p_id]->gen) ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                if (r_id != -1) {
                    room[r_id].num_patients_allocated++;
                    mandatory_patients[p_id]->admission_day = day;
                    mandatory_patients[p_id]->assigned_ot = assigned_ot;
                    mandatory_patients[p_id]->assigned_room_no = r_id;
                }
                else
                    // put the patient in the PQ
                    insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id]->surgery_due_day, 0));
            }
            else
                if (day < mandatory_patients[p_id]->surgery_due_day) {
                    insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id]->surgery_due_day, 0));
                    continue;
                }
        }
    }
    return unscheduled_mandatory;
}

//----------------------------------------------------ABOVE: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

//-----------------------------------------------------------BELOW: GENETIC ALGORITHM-----------------------------------------------------------------

//const int POPULATION_SIZE = 100, NUM_ITER = 10000, CONVERGENCE_STOPPING_CRITERION = 100;
//int** POPULATION, * G_BEST, CHROMOSOME_SIZE;
//int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
//int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, *chromosome;

//-------------------------------------------------------BELOW: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm(void);
int evaluateFitnessScore(int* chromosome);
void orderCrossover(void);
void swapMutation(void);
void generateNewChromosome(int chromo_num);
void generatePopulation(void);
void crossoverTournamentSelection(void);
void mutationTournamentSelection(void);
void crossoverElitism(void);
void mutationElitism(void);
void initDataStructures(void);
void freeDataStructures(void);
void printPopulation(void);

//-------------------------------------------------------ABOVE: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm(void)
{
    int i, j, best_fitness, g_best;
    unsigned int same_fitness_iter;
    float p_c = 0.8;

    generatePopulation();
    for (i = 0; i < POPULATION_SIZE; ++i)
        POPULATION[i][CHROMOSOME_SIZE] = evaluateFitnessScore(POPULATION[i]);
    memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE + 1) * sizeof(int));
    g_best = G_BEST[CHROMOSOME_SIZE];
    printPopulation();

    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    for (i = 0, same_fitness_iter = 0; i != NUM_ITER && same_fitness_iter != CONVERGENCE_STOPPING_CRITERION; ++i) {
        if ((rand() / (float)RAND_MAX) <= p_c) {
            crossoverTournamentSelection();
            orderCrossover();
            // calculate the fitness of the 2 new offsprings
            CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE] = evaluateFitnessScore(CROSSOVER_OFFSPRING_STORAGE_PLACE[0]);
            CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE] = evaluateFitnessScore(CROSSOVER_OFFSPRING_STORAGE_PLACE[1]);
            crossoverElitism();
            for (j = 0; j < 2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE + 1));
        }
        else {
            mutationTournamentSelection();
            swapMutation();
            // calculate the fitness of the new offspring
            MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] = evaluateFitnessScore(MUTATED_OFFSPRING_STORAGE_PLACE);
            mutationElitism();
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 1));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        best_fitness = G_BEST[CHROMOSOME_SIZE];
        if (best_fitness == g_best)
            same_fitness_iter++;
        else
            same_fitness_iter = 0;
        g_best = G_BEST[CHROMOSOME_SIZE];
    }
    puts("\nChecking whether the iterations were over OR the algorithm converged: ");
    printf("Number of iterations: %d\tsame_fitness_iter: %d", i, same_fitness_iter);
    // either i will be NUM_ITER (10000) OR same_fitness_iter will be CONVERGENCE_STOPPING_CRITERION (100).
    // G_BEST is the best chromosome - use it for admitting patients.
}

int evaluateFitnessScore(int* chromosome, PriorityQueue* pq)
{   // fitness score is the number of mandatory patients who were admitted in the scheduling period.
    // The best fitness score is CHROMOSOME_SIZE
    int unscheduled_mandatory, total_unscheduled_mandatory;

    unscheduled_mandatory = admit_patients(room_gender_map, pq, chromosome);
    total_unscheduled_mandatory = unscheduled_mandatory + pq->current_size;

    // FITNESS = MANDATORY PATIENTS WHO WERE ADMITTED DURING THE SCHEDULING PERIOD
    return (mandatory_count - total_unscheduled_mandatory);
}

void orderCrossover(void)
{   // Apply Denver's Order Crossover on the two crossover_parents and generate two new offsprings.
    int r1, r2, i;

    // Select two random crossover points
    do {
        r1 = rand() % CHROMOSOME_SIZE;
        r2 = rand() % CHROMOSOME_SIZE;
    } while (r1 == r2);

    if (r1 > r2) {
        r1 += r2;
        r2 = r1 - r2;
        r1 -= r2;
    }

    // Copy parents to offspring storage directly
    memcpy(CROSSOVER_OFFSPRING_STORAGE_PLACE[0], CROSSOVER_PARENT_STORAGE_PLACE[0], (CHROMOSOME_SIZE + 1) * sizeof(int));
    memcpy(CROSSOVER_OFFSPRING_STORAGE_PLACE[1], CROSSOVER_PARENT_STORAGE_PLACE[1], (CHROMOSOME_SIZE + 1) * sizeof(int));

    // Swap genes outside the crossover segment
    for (i = 0; i < CHROMOSOME_SIZE; ++i) {
        if (i >= r1 && i <= r2)
            continue;
        CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = CROSSOVER_PARENT_STORAGE_PLACE[1][i];
        CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = CROSSOVER_PARENT_STORAGE_PLACE[0][i];
    }
}

void swapMutation(void)
{   // take the offspring from MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2;

    do {
        r1 = rand() % CHROMOSOME_SIZE;
        r2 = rand() % CHROMOSOME_SIZE;
    } while (r1 == r2);

    MUTATED_OFFSPRING_STORAGE_PLACE[r1] += MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r2] = MUTATED_OFFSPRING_STORAGE_PLACE[r1] - MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= MUTATED_OFFSPRING_STORAGE_PLACE[r2];
}

void swapGenes(int chromo_num, int r1, int r2)
{
    POPULATION[chromo_num][r1] += POPULATION[chromo_num][r2];
    POPULATION[chromo_num][r2] = POPULATION[chromo_num][r1] - POPULATION[chromo_num][r2];
    POPULATION[chromo_num][r1] -= POPULATION[chromo_num][r2];
}

void generateNewChromosome(int chromo_num)
{
    int j, r1, r2;


    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
    for (j = 0; j < CHROMOSOME_SIZE; ++j)
        POPULATION[chromo_num][j] = POPULATION[chromo_num - 1][j];

    for (j = 0; j < CHROMOSOME_SIZE; ++j) {
        do {
            r1 = rand() % (CHROMOSOME_SIZE);
            r2 = rand() % (CHROMOSOME_SIZE);
        } while (r1 == r2);

        swapGenes(chromo_num, r1, r2);
    }
}

void generatePopulation(void)
{
    int i, j;

    // Copy mandatory patients' IDs into the first chromosome
    for (j = 0; j < CHROMOSOME_SIZE; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;

    // Generate new chromosomes for the rest
    for (i = 1; i < POPULATION_SIZE; ++i)
        generateNewChromosome(i);
}

void crossoverTournamentSelection(void)
{   // select 2 parents using Tournament Selection method
    int r11, r12, r13, r21, r22, r23, f11, f12, f13, f21, f22, f23;
    int best_fitness, best_fitness_idx1, best_fitness_idx2;

    // select first parent
    do {
        r11 = rand() % (POPULATION_SIZE);
        r12 = rand() % (POPULATION_SIZE);
        r13 = rand() % (POPULATION_SIZE);
    } while (r11 == r12 || r12 == r13 || r11 == r13);

    // select the chromosome1 with the best fitness
    f11 = POPULATION[r11][CHROMOSOME_SIZE];
    f12 = POPULATION[r12][CHROMOSOME_SIZE];
    f13 = POPULATION[r13][CHROMOSOME_SIZE];
    best_fitness = f11;
    best_fitness_idx1 = r11;

    if (f12 > best_fitness) {
        if (f13 > f12) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }
        else {
            best_fitness = f12;
            best_fitness_idx1 = r12;
        }
    }
    else
        if (f13 > best_fitness) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }


    // select second parent
    do {
        r21 = rand() % (POPULATION_SIZE);
        r22 = rand() % (POPULATION_SIZE);
        r23 = rand() % (POPULATION_SIZE);
    } while (r21 == r22 || r22 == r23 || r21 == r23 ||
        r11 == r21 || r11 == r22 || r11 == r23 ||
        r12 == r21 || r12 == r22 || r12 == r23 ||
        r13 == r21 || r13 == r22 || r13 == r23);

    // select the chromosome2 with the best fitness
    f21 = POPULATION[r21][CHROMOSOME_SIZE];
    f22 = POPULATION[r22][CHROMOSOME_SIZE];
    f23 = POPULATION[r23][CHROMOSOME_SIZE];
    best_fitness = f21;
    best_fitness_idx2 = r21;

    if (f22 > best_fitness) {
        if (f23 > f22) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
        else {
            best_fitness = f22;
            best_fitness_idx2 = r22;
        }
    }
    else
        if (f23 > best_fitness) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[i], POPULATION[best_fitness_idx1], (CHROMOSOME_SIZE + 1) * sizeof(int));
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[1], POPULATION[best_fitness_idx2], (CHROMOSOME_SIZE + 1) * sizeof(int));
}

void mutationTournamentSelection(void)
{   // select 2 parents using Tournament Selection method
    int r11, r12, r13, f11, f12, f13;
    int best_fitness, best_fitness_idx;

    // select first parent
    do {
        r11 = rand() % (POPULATION_SIZE);
        r12 = rand() % (POPULATION_SIZE);
        r13 = rand() % (POPULATION_SIZE);
    } while (r11 == r12 || r12 == r13 || r11 == r13);

    // select the chromosome1 with the best fitness
    f11 = POPULATION[r11][CHROMOSOME_SIZE];
    f12 = POPULATION[r12][CHROMOSOME_SIZE];
    f13 = POPULATION[r13][CHROMOSOME_SIZE];
    best_fitness = f11;
    best_fitness_idx = r11;

    if (f12 > best_fitness) {
        if (f13 > f12) {
            best_fitness = f13;
            best_fitness_idx = r13;
        }
        else {
            best_fitness = f12;
            best_fitness_idx = r12;
        }
    }
    else
        if (f13 > best_fitness) {
            best_fitness = f13;
            best_fitness_idx = r13;
        }

    memcpy(MUTATE_PARENT_STORAGE_PLACE, POPULATION[best_fitness_idx], (CHROMOSOME_SIZE + 1) * sizeof(int));
}

void crossoverElitism(void)
{
    int i, worst_fitness_chromosome_index = 0, second_worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
            worst_fitness_chromosome_index = i;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE] < POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
        memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE + 1));
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE] > POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE])
        memcpy(POPULATION[second_worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE + 1));
}

void mutationElitism(void)
{
    int i, worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
        memcpy(POPULATION[worst_fitness_chromosome_index], MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 1));
}

void initDataStructures(void)
{
    MUTATED_OFFSPRING_STORAGE_PLACE = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    MUTATE_PARENT_STORAGE_PLACE = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    if (!MUTATED_OFFSPRING_STORAGE_PLACE || !MUTATE_PARENT_STORAGE_PLACE)
        ASSERT(0, "Dynamic Memory Allocation Error for MUTATED_STORAGE_PLACE");

    CROSSOVER_OFFSPRING_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    CROSSOVER_OFFSPRING_STORAGE_PLACE[0] = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    CROSSOVER_OFFSPRING_STORAGE_PLACE[1] = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    if (!CROSSOVER_OFFSPRING_STORAGE_PLACE || !CROSSOVER_OFFSPRING_STORAGE_PLACE[0] || !CROSSOVER_OFFSPRING_STORAGE_PLACE[1])
        ASSERT(0, "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE");

    CROSSOVER_PARENT_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    CROSSOVER_PARENT_STORAGE_PLACE[0] = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    CROSSOVER_PARENT_STORAGE_PLACE[1] = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    if (!CROSSOVER_PARENT_STORAGE_PLACE || !CROSSOVER_PARENT_STORAGE_PLACE[0] || !CROSSOVER_PARENT_STORAGE_PLACE[1])
        ASSERT(0, "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE");

    G_BEST = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    if (!G_BEST)
        ASSERT(0, "DYNAMIC MEMORY ALLOCATION ERROR FOR G_BEST");

    POPULATION = (int**)calloc(POPULATION_SIZE, sizeof(int*));
    if (!POPULATION)
        ASSERT(0, "DYNAMIC MEMORY ALLOCATION ERROR FOR POPULATION");

    for (int i = 0; i < POPULATION_SIZE; ++i) {
        POPULATION[i] = (int*)calloc(CHROMOSOME_SIZE + 1, sizeof(int));
        if (!POPULATION[i])
            ASSERT(0, "DYNAMIC MEMORY ALLOCATION ERROR FOR A CHROMOSOME");

        // Set last column to -1
        POPULATION[i][CHROMOSOME_SIZE] = -1;
    }
}

void freeDataStructures(void)
{
    free(MUTATED_OFFSPRING_STORAGE_PLACE);
    free(MUTATED_OFFSPRING_STORAGE_PLACE);

    free(CROSSOVER_OFFSPRING_STORAGE_PLACE[0]);
    free(CROSSOVER_OFFSPRING_STORAGE_PLACE[1]);
    free(CROSSOVER_OFFSPRING_STORAGE_PLACE);

    free(CROSSOVER_PARENT_STORAGE_PLACE[0]);
    free(CROSSOVER_PARENT_STORAGE_PLACE[1]);
    free(CROSSOVER_PARENT_STORAGE_PLACE);

    free(G_BEST);

    for (int i = 0; i < POPULATION_SIZE; ++i)
        free(POPULATION[i]);
    free(POPULATION);
}

void printPopulation(void)
{
    int i, j;
    for (i = 0; i < POPULATION_SIZE; ++i) {
        for (j = 0; j < CHROMOSOME_SIZE; ++j)
            printf("%d ", POPULATION[i][j]);
        putch('\n');
    }
}

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

int main(void) {
    parse_json("C:/sant_saran/C in One/IHTC/VS_Code/EA/test09.json");
    PriorityQueue* pq;
    srand(time(0));
    initDataStructures();
    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
    if (!pq)
        ASSERT(0, "Dynamic Memory Allocation Error for PQ");

    CHROMOSOME_SIZE = mandatory_count;
    initialize_room_gender_map(&room_gender_map);
    initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
    populate_room_gender_map(room_gender_map);
    // printf("\nroom: 6\tgender: %d", room_gender_map[6]);
    //print_map(room_gender_map);

    //sort_mandatory_patients_on_release_day(mandatory_patients, mandatory_count);
    //sort_mandatory_patients_on_LOS();
    //print_sorted_mandatory_array();
    //sort_optional_patients_on_release_day(optional_patients, optional_count);
    //append_optional_to_mandatory(sorted_mandatory_patients, sorted_optional_patients);
    //print_sorted_optional_array();
    //print_ots(ot);
    admit_patients(room_gender_map, pq, chromosome);
    applyGeneticAlgorithm();
    freeDataStructures();

    /*
   create_dm_nurses_availability();
   sorting_nurse_id_max_load();
   create_3d_array();
   print_room_schedule();
   initialize_rooms_req(num_rooms);
   create_rooms_req();
  print_rooms_req();
   nurse_assignments();

  // print_dm_nurses();

  // print_mandatory_patients();
    //print_sorted_mandatory_array();
    //print_sorted_mandatory_patients();
  create_json_file(patients , num_patients , nurses , num_nurses,num_rooms, "i05","D:/major_code/build/output");
   // Use the parsed data in your algorithm
   // int *surgery_time[num_surgeons][days];
   // printf("Weights:\n");
   // printf("Room Mixed Age: %d\n", weights->room_mixed_age);
   // printf("Room Nurse Skill: %d\n", weights->room_nurse_skill);
   // printf("Continuity of Care: %d\n", weights->continuity_of_care);
   // printf("Nurse Excessive Workload: %d\n", weights->nurse_excessive_workload);
   // printf("Open Operating Theater: %d\n", weights->open_operating_theater);
   // printf("Surgeon Transfer: %d\n", weights->surgeon_transfer);
   // printf("Patient Delay: %d\n", weights->patient_delay);
   // printf("Unscheduled Optional: %d\n", weights->unscheduled_optional);

   // print_surgeons(surgeon);
   //print_ots(ot);
   // print_rooms();
    print_nurses();
   // print_patients(patients);
   // print_occupants();
   // print_mandatory_patients();
   // print_optional_patients();


   //Free allocated memory
  // free_patients_sorted_array(sorted_mandatory_patients);
   //free_patients_sorted_array(sorted_optional_patients);
   free_occupants();
   free_patients();
   free_surgeons();
   free_ots();
   free_rooms();
   //free_nurses();
   free(weights);

   */

    freePopulation();
    return 0;
}

