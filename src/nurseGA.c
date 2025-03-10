#include <definition.h>

// Extern declarations (telling the compiler these are defined elsewhere)
typedef struct {
    HeapNode** data;
    int size;
    int capacity;
} Mand_opt_PQ;

int last_swap_pos1 = -1, last_swap_pos2 = -1;

#define NUM_THREADS 4
#define CHECK_INTERVAL 120 // Pause every 2 minutes (120 seconds)

typedef struct {
    int* G_BEST;
    PriorityQueue* pq;
    int thread_id;
} ThreadData;

pthread_mutex_t lock;
int* global_best_solution;
int global_best_fitness = INT_MAX;
int stop_threads = 0; // Flag to stop threads

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
extern int* room_gender_map;
//extern RoomVector* v_A, * v_B, * v_empty;

extern const int POPULATION_SIZE, NUM_ITER, CONVERGENCE_STOPPING_CRITERION;
extern int** POPULATION, * G_BEST;
extern int size; int CHROMOSOME_SIZE_NEW_GA = 0;
extern int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
extern int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, * chromosome;

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
extern void append_optionals();
extern void put_occupants();
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
//extern void init_Rooms(RoomVector* vector, int initialcap);
//extern void pushback(RoomVector* vector, Rooms* room);
//extern Rooms* removebyid(RoomVector* vector, int room_id);
//extern void moveRoom(RoomVector* source, RoomVector* destination, int room_id);
//extern void freevector(RoomVector* vector);
//extern void make_3_vectors(int** room_gender_map);
//extern int find_max_surgeon_id(Node* head);
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
extern void empty_pq(PriorityQueue* pq);
extern int** room_shift_nurse;
extern void initialize_room_shift_nurse();
extern int compare_ots(const void* a, const void* b);
extern int sorting_day;
extern void print_surgeons(Surgeon*);
extern void nurse_assignment_new(void);
//extern OTs* admitFromPQ_GA(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt, int* fitness_storage_place);

// --------------------ABOVE: Functions for admitting optional patients from the PQ (linked list functions as well)---------------------

//-------------------------------------------------BELOW: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

extern int k;
extern int compare_release_Day(const void* a, const void* b);


void update_num_patients_info(void);

void reset_valuesNewGA(void) {
    //reset the patient structure values
    //reset rooms structure values.
    //reset the surgeon structure values.
    //reset the OT structure values.
    //reset the nurse structure values.
    //reset the occupants structure values.


    //patient_structure
    for (int i = 0; i < num_patients; i++) {
        patients[i].is_admitted = 0;
        patients[i].assigned_ot = -1;
        patients[i].assigned_room_no = -1;
        patients[i].admission_day = -1;

    }

    //room_structure
    for (int i = 0; i < num_rooms; i++) {
        room[i].num_patients_allocated = 0;
        room[i].occupants_cap = 0;
        for (int j = 0; j < days; ++j) {
            room[i].num_patients_info[j] = 0;
            room[i].gender_days_info[j] = -1;
        }
    }
    //assign_occupants_to_rooms();
    update_num_patients_info();

    //surgeon_structure
    for (int i = 0; i < num_surgeons; i++) for (int j = 0; j < days; j++) surgeon[i].time_left[j] = surgeon[i].max_surgery_time[j];

    //OT_structure
    for (int i = 0; i < num_ots; i++) for (int j = 0; j < days; j++) ot[i].time_left[j] = ot[i].max_ot_time[j];
}

void printOtDaysDataArr(OTs*** arr)
{
    int d, i;
    for (d = 0; d < days; ++d) {
        putchar('\n');
        for (i = 0; i < num_ots; ++i)
            printf("id: %d\tmax_ot_time: %d", arr[d][i]->id, arr[d][i]->max_ot_time[d]);
    }


}


void sort_ot_days_data_arr(OTs*** ot_days_data_arr, int day) {
    if (ot_days_data_arr == NULL || num_ots <= 0 || day < 0 || day >= days) {
        fprintf(stderr, "Error: Invalid input array or day\n");
        return;
    }
    sorting_day = day;
    qsort(ot_days_data_arr[day], num_ots, sizeof(OTs*), compare_ots);
}



int admitPatientsNewGA(int** room_gender_map, PriorityQueue* pq, int* chromosome, int size) {
    int i, j, p_id, s_id, r_id, d, ot_id, max = 0;
    int admitted_mandatory_count = 0, current_ot_index = 0;
    int unscheduled_mandatory = 0, scheduled_optional = 0, scheduled_mandatory = 0;

    // Allocate memory for OT scheduling data
    OTs** ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    OTs*** ot_days_data_arr = (OTs***)calloc(days, sizeof(OTs**)), * current_ot;

    if (!ot_data_arr || !ot_days_data_arr) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(ot_data_arr);
        free(ot_days_data_arr);
        return EXIT_FAILURE;
    }

    for (i = 0; i < num_ots; ++i) {
        ot_data_arr[i] = ot + i;
    }

    for (i = 0; i < days; ++i) {
        ot_days_data_arr[i] = (OTs**)calloc(num_ots, sizeof(OTs*));
        if (!ot_days_data_arr[i]) {
            fprintf(stderr, "Memory allocation failed for ot_days_data_arr[%d].\n", i);
            for (j = 0; j < i; j++) free(ot_days_data_arr[j]);
            free(ot_days_data_arr);
            free(ot_data_arr);
            exit(EXIT_FAILURE);
        }
        memcpy(ot_days_data_arr[i], ot_data_arr, num_ots * sizeof(OTs*));
        sort_ot_data_arr(ot_days_data_arr[i], i);
    }
    //printOtDaysDataArr(ot_days_data_arr);
    for (int i = 0; i < size; i++)
        if (max < chromosome[i])
            max = chromosome[i];
    // Allocate memory for patient scheduling tracking
    int* unscheduled_mandatory_patients = (int*)calloc(max + 1, sizeof(int));
    int* scheduled_optional_patients = (int*)calloc(num_patients, sizeof(int));
    int* scheduled_mandatory_patients = (int*)calloc(num_patients, sizeof(int));

    if (!unscheduled_mandatory_patients || !scheduled_optional_patients || !scheduled_mandatory_patients) {
        fprintf(stderr, "Memory allocation failed for patient arrays.\n");
        free(unscheduled_mandatory_patients);
        free(scheduled_optional_patients);
        free(scheduled_mandatory_patients);
        for (i = 0; i < days; ++i) free(ot_days_data_arr[i]);
        free(ot_days_data_arr);
        free(ot_data_arr);
        return EXIT_FAILURE;
    }

    // ------------------------------------------------------------ START SCHEDULING ------------------------------------------------------------

    for (int p = 0; p < size; ++p) {
        p_id = chromosome[p];
        s_id = patients[p_id].surgeon_id;
        //gender g = patients[p_id].gen;

        for (d = patients[p_id].surgery_release_day; d < days; ++d) {
            if (d > patients[p_id].surgery_due_day && patients[p_id].mandatory) break;
            if (surgeon[s_id].time_left[d] < patients[p_id].surgery_duration) continue;

            // Find an available OT with sufficient time
            current_ot_index = 0;
            current_ot = ot_days_data_arr[d][current_ot_index];
            ot_id = ot_days_data_arr[d][current_ot_index]->id;
            while (current_ot_index < num_ots && (current_ot->time_left[d] < patients[p_id].surgery_duration)) {
                ot_id = ot_days_data_arr[d][current_ot_index]->id;
                current_ot = ot_days_data_arr[d][current_ot_index];
                current_ot_index++;

                if (current_ot_index >= num_ots) {
                    //printf("Breaking: current_ot_index reached num_ots\n");
                    break;
                }
            }


            //if (current_ot_index == num_ots) continue; // No OT available

            // Find a suitable room
            r_id = findSuitableRoomNewGA(p_id, d);
            if (r_id == -1) continue; // No suitable room found

            // Double-check OT availability before finalizing assignment
            if (ot[ot_id].time_left[d] < patients[p_id].surgery_duration) continue;

            // Allocate patient
            for (int k = d; k < d + patients[p_id].length_of_stay && k < days; k++) {
                room[r_id].num_patients_info[k]++;
                room[r_id].gender_days_info[k] = patients[p_id].gen;
            }
            room[r_id].num_patients_allocated++;
            room[r_id].gen = patients[p_id].gen;
            patients[p_id].admission_day = d;
            patients[p_id].assigned_ot = ot_id;
            patients[p_id].assigned_room_no = r_id;
            patients[p_id].is_admitted = 1;
            surgeon[s_id].time_left[d] -= patients[p_id].surgery_duration;
            ot[ot_id].time_left[d] -= patients[p_id].surgery_duration;
            // ot_days_data_arr[d][current_ot_index]->time_left[d] -= patients[p_id].surgery_duration;

            break; // Move to the next patient after scheduling
        }
    }

    // Count scheduled and unscheduled patients
    for (i = 0; i < max; i++) {
        if (patients[i].admission_day == -1 && patients[i].mandatory) {
            unscheduled_mandatory++;
            unscheduled_mandatory_patients[i] = 1;
            continue;
        }
        if (patients[i].mandatory) {
            scheduled_mandatory++;
            scheduled_mandatory_patients[patients[i].id] = 1;
            continue;
        }
        if (patients[i].admission_day != -1 && !patients[i].mandatory) {
            scheduled_optional++;
            scheduled_optional_patients[patients[i].id] = 1;
        }
    }
    /*printf("\nUnscheduled_mandatory:");
    for (int i = 0; i < max; i++) {
        if (unscheduled_mandatory_patients[i] == 1) printf("%d\t", i);
    }*/

    // Free allocated memory
    //free(unscheduled_mandatory_patients);
    //free(scheduled_mandatory_patients);
    //free(scheduled_optional_patients);

    for (i = 0; i < days; ++i) free(ot_days_data_arr[i]);
    free(ot_days_data_arr);
    free(ot_data_arr);

    return unscheduled_mandatory;
}

/*
void updateResources(int d) {
    int i, r_id;
    gender g;

    for (r_id=0; r<num_rooms; ++r) {
        g = room[r_id].gen;
        if (!room[r_id].num_patients_info[d] && g != UNKNOWN) {
            g ? moveRoom(v_B, v_empty, r_id) : moveRoom(v_A, v_empty, r_id);
            room[r_id].gen = -1;
        }
    }
}
*/

int findSuitableRoomNewGA(int p_id, int d) {
    gender g = patients[p_id].gen;
    char* age = patients[p_id].age_group;
    int room_to_be_assigned = -1;

    // Check non-empty rooms (already assigned gender)
    for (int i = 0; i < num_rooms; i++) {
        int flag = 0;
        // Check room's gender for each day of the patient's stay
        for (int j = d; j < d + patients[p_id].length_of_stay && j < days; j++) {
            if (room[i].gender_days_info[j] != g) {
                flag = 1;
                break;
            }
        }
        if (flag) continue;

        // Check age group compatibility
       // if (strcmp(room[i].age_group, age) != 0) continue;

        // Check room capacity for each day of stay
        for (int j = d; j < d + patients[p_id].length_of_stay && j < days; j++) {
            if (room[i].cap <= room[i].num_patients_info[j]) {
                flag = 1;
                break;
            }
        }
        if (flag) continue;

        // Check incompatible rooms
        for (int k = 0; k < patients[p_id].num_incompatible_rooms; k++) {
            if (room[i].id == patients[p_id].incompatible_room_ids[k]) {
                flag = 1;
                break;
            }
        }
        if (flag) continue;

        return room[i].id; // Suitable room found
    }

    // Check empty rooms (gender not set)
    for (int i = 0; i < num_rooms; i++) {
        int flag = 0;
        // Check if room is empty (gender -1) for all days of stay
        for (int j = d; j < d + patients[p_id].length_of_stay && j < days; j++) {
            if (room[i].gender_days_info[j] != -1 && room[i].gender_days_info[j] != g) {
                flag = 1;
                break;
            }
        }
        if (flag) continue;

        // Check age group compatibility
       // if (strcmp(room[i].age_group, age) != 0) continue;

        // Check capacity for each day
        for (int j = d; j < d + patients[p_id].length_of_stay && j < days; j++) {
            if (room[i].cap <= room[i].num_patients_info[j]) {
                flag = 1;
                break;
            }
        }
        if (flag) continue;

        // Check incompatible rooms
        for (int k = 0; k < patients[p_id].num_incompatible_rooms; k++) {
            if (room[i].id == patients[p_id].incompatible_room_ids[k]) {
                flag = 1;
                break;
            }
        }
        if (flag) continue;

        // Assign room and set gender for all days
        for (int j = d; j < d + patients[p_id].length_of_stay && j < days; j++) {
            room[i].gender_days_info[j] = g;
        }
        return room[i].id;
    }

    return -1; // No suitable room found
}

//int findSuitableRoomNewGA(int p_id, int d) {
//    // Returns a room_id OR -1 if no suitable room is found
//    int j, r_id = -1;
//    gender g = patients[p_id].gen;
//    char* age = patients[p_id].age_group;
//
//    for (int i = 0; i < num_rooms; i++) {
//        r_id = room[i].id;
//
//        // Check if the room is already full on the admission day
//        if (room[r_id].cap <= room[r_id].num_patients_info[d]) continue;
//
//        // Check for incompatible rooms
//        int is_incompatible = 0;
//        for (j = 0; j < patients[p_id].num_incompatible_rooms; j++) {
//            if (r_id == patients[p_id].incompatible_room_ids[j]) {
//                is_incompatible = 1; // Mark as incompatible
//                break;
//            }
//        }
//        if (is_incompatible) continue;
//
//        // Check gender compatibility
//        if (g != room[r_id].gen && room[r_id].gen != -1) continue;
//
//        // Check availability across the patient's entire stay
//        int valid_room = 1;
//        for (j = d; j <= d + patients[p_id].length_of_stay && j < days; ++j) {
//            if (room[r_id].cap == room[r_id].num_patients_info[j]) {
//                valid_room = 0;
//                break;
//            }
//        }
//        if (!valid_room) continue;
//
//        return r_id; // Found a suitable room
//    }
//
//    return -1; // No suitable room found
//}

    // look in empty rooms
    /*
    if (r_id == -1) {
        for (i = 0; i < v_empty->size; i++) {
            flag = 0;
            r_id = v_empty->data[i]->id;
            // Check if room is full
            if ((room[r_id].cap <= room[r_id].num_patients_info[d])
                continue;
            // Check for incompatible rooms
            for (j = 0; j < patients[p_id].num_incompatible_rooms; j++) {
                if (r_id == patients[p_id].incompatible_room_ids[j]) {
                    flag = 1; // Mark as incompatible
                    break;
                }
            }
            // If incompatible, skip this room
            if (flag) continue;
            // Move room from empty list to the main vector
            moveRoom(v_empty, vector, r_id);
            room[r_id].gen = g;
            return r_id; // Found a suitable room
        }
    }
    */


    //----------------------------------------------------ABOVE: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

    //-----------------------------------------------------------BELOW: GENETIC ALGORITHM-----------------------------------------------------------------

    //-------------------------------------------------------BELOW: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

    // all taken from ihtc_ga.c
extern void orderCrossover(void);
extern void swapMutation(void);
extern void swapGenes(int, int, int);
extern void generateNewChromosome(int);
extern void initDataStructures(void);
extern void freeDataStructures(void);
extern void printPopulation(void);

void applyGeneticAlgorithmNewGA(PriorityQueue* pq);
void evaluateFitnessScoreNewGA(int* chromosome, PriorityQueue* pq, int s);
void crossoverTournamentSelectionNewGA(void);
void mutationTournamentSelectionNewGA(void);
void generatePopulationNewGA(void);
void mutationElitismViolations(void);
void crossoverElitismViolations(void);
void mutationElitismCost(void);
void crossoverElitismCost(void);
void minimizeCost(PriorityQueue*); // returns the minimized cost
const int NUM_COST_ITER = 1000000;
void orderCrossoverNewGA(void);
void printPopulationNewGA(void);
void swapMutationNewGA(void);
void generateNewChromosomeNewGA(int chromo_num);

int findViolations(void);
int findCost(void);
int patient_delay();
int elective_unscheduled_patients();
int open_operating_theatre();
int room_age_mix();
int skill_level_mix();



//-------------------------------------------------------ABOVE: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------


void printpopulationnewga(void)
{
    int i, j;
    for (i = 0; i < POPULATION_SIZE; ++i) {
        printf("Chromosome %d: ", i + 1);
        for (j = 0; j < size; ++j)
            printf("%d ", POPULATION[i][j]);
        putchar('\n');
    }
}



void orderCrossoverNewGA(void) {
    int r1, r2, i, k, m;
    int max = 0;
    if (CHROMOSOME_SIZE_NEW_GA == 0) return;

    for (int i = 0; i < CHROMOSOME_SIZE_NEW_GA; i++) {
        if (max < CROSSOVER_PARENT_STORAGE_PLACE[0][i])
            max = CROSSOVER_PARENT_STORAGE_PLACE[0][i];
        if (max < CROSSOVER_PARENT_STORAGE_PLACE[1][i])
            max = CROSSOVER_PARENT_STORAGE_PLACE[1][i];
    }

    bool* visited1 = (bool*)calloc(max + 1, sizeof(bool));
    bool* visited2 = (bool*)calloc(max + 1, sizeof(bool));

    if (!visited1 || !visited2) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    do {
        r1 = rand() % CHROMOSOME_SIZE_NEW_GA;
        r2 = rand() % CHROMOSOME_SIZE_NEW_GA;
    } while (r1 == r2);

    if (r1 > r2) {
        int temp = r1;
        r1 = r2;
        r2 = temp;
    }

    for (i = 0; i < size; i++) {
        CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = -1;
        CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = -1;
    }

    for (i = r1; i <= r2; i++) {
        int gene1 = CROSSOVER_PARENT_STORAGE_PLACE[0][i];
        int gene2 = CROSSOVER_PARENT_STORAGE_PLACE[1][i];

        CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = gene1;
        CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = gene2;

        visited1[gene1] = true;
        visited2[gene2] = true;
    }

    k = (r2 + 1) % CHROMOSOME_SIZE_NEW_GA;
    m = k;

    for (i = 0; i < CHROMOSOME_SIZE_NEW_GA; i++) {
        int index = (r2 + 1 + i) % CHROMOSOME_SIZE_NEW_GA;

        int gene1 = CROSSOVER_PARENT_STORAGE_PLACE[1][index];
        if (!visited1[gene1]) {
            if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] != -1) {
                k = (k + 1) % CHROMOSOME_SIZE_NEW_GA;
            }
            CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] = gene1;
            visited1[gene1] = true;
        }

        int gene2 = CROSSOVER_PARENT_STORAGE_PLACE[0][index];
        if (!visited2[gene2]) {
            if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] != -1) {
                m = (m + 1) % CHROMOSOME_SIZE_NEW_GA;
            }
            CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] = gene2;
            visited2[gene2] = true;
        }
    }

    CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA] = -1;
    CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA + 1] = -1;
    CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA] = -1;
    CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA + 1] = -1;

    free(visited1);
    free(visited2);
}

void swapMutationNewGA(void)
{   // take the offspring from MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2;
    if (size == 0)return;
    for (int i = 0; i < size; i++) {
        MUTATED_OFFSPRING_STORAGE_PLACE[i] = MUTATE_PARENT_STORAGE_PLACE[i];
    }

    do {
        r1 = rand() % size;
        r2 = rand() % size;
    } while (r1 == r2);

    MUTATED_OFFSPRING_STORAGE_PLACE[r1] += MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r2] = MUTATED_OFFSPRING_STORAGE_PLACE[r1] - MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= MUTATED_OFFSPRING_STORAGE_PLACE[r2];

    MUTATED_OFFSPRING_STORAGE_PLACE[size] = -1;
    MUTATED_OFFSPRING_STORAGE_PLACE[size + 1] = -1;
}

//void generateNewChromosomeNewGA(int chromo_num)
//{
//    int j, r1, r2;
//
//    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
//    for (j = 0; j < CHROMOSOME_SIZE_NEW_GA; ++j)
//        POPULATION[chromo_num][j] = POPULATION[chromo_num - 1][j];
//
//    for (j = 0; j < CHROMOSOME_SIZE_NEW_GA; ++j) {
//        do {
//            r1 = rand() % (CHROMOSOME_SIZE_NEW_GA);
//            r2 = rand() % (CHROMOSOME_SIZE_NEW_GA);
//        } while (r1 == r2);
//
//        swapGenes(chromo_num, r1, r2);
//    }
//}


void update_num_patients_info() {
    int i, r_id, j;
    for (i = 0; i < num_occupants; i++) {
        r_id = occupants[i].room_id;
        for (j = 0; j <= occupants[i].length_of_stay; j++) {
            room[r_id].num_patients_info[j]++;
            room[r_id].gender_days_info[j] = occupants[i].gen;
        }
    }
}

void applyGeneticAlgorithmNewGA(PriorityQueue* pq)
{
    int i, j, best_fitness = INT_MAX, g_best = INT_MAX, best_fitness_mandatory = 0, minimized_cost = 0, feasible = 0;
    unsigned int same_fitness_iter = 0;
    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
    float p_m = 0.15;
    int last_improvement = 0;
    int last_improvement_iter = 0;

    generatePopulationNewGA();
    evaluateFitnessScoreNewGA(POPULATION[0], pq, mandatory_count);
    memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
    for (i = 0; i < POPULATION_SIZE; ++i) {
        evaluateFitnessScoreNewGA(POPULATION[i], pq, mandatory_count);
        if (G_BEST[CHROMOSOME_SIZE_NEW_GA] > POPULATION[i][CHROMOSOME_SIZE_NEW_GA]) {
            memcpy(G_BEST, POPULATION[i], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
            //    last_improvement = i;
            //    //break;
            //    /* printf("\nImprovement in population:%d", last_improvement);
            //     printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
        }
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] == 0) {
            feasible = 1;  // Mark that at least one feasible solution has been found
            if (G_BEST[CHROMOSOME_SIZE_NEW_GA] != 0)
                memcpy(G_BEST, POPULATION[i], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));

            if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]) {
                memcpy(G_BEST, POPULATION[i], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
                last_improvement = i;
                /*printf("\nImprovement in population:%d", last_improvement);
                printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
            }
        }
    }

    for (i = 0; i < NUM_ITER; ++i) {
        //if (G_BEST[CHROMOSOME_SIZE_NEW_GA] == 0) break;
        if ((rand() / (float)RAND_MAX) <= p_c) {
            crossoverTournamentSelectionNewGA();
            orderCrossoverNewGA();
            for (j = 0; j < 2; ++j)
                evaluateFitnessScoreNewGA(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq, mandatory_count);
            crossoverElitismViolations();
            for (j = 0; j < 2; ++j) {
                if (G_BEST[CHROMOSOME_SIZE_NEW_GA] > CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA])
                {
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
                    last_improvement_iter = i;
                    //break;
                    /* printf("\nImprovement at iteration:%d", last_improvement_iter);
                     printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
                }
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA] == 0) {
                    feasible = 1;
                    if (G_BEST[CHROMOSOME_SIZE_NEW_GA] != 0)
                        memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
                    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]) {
                        memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
                        last_improvement_iter = i;
                        //break;
                        /* printf("\nImprovement at iteration:%d", last_improvement_iter);
                         printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
                    }
                }
            }
        }
        else {
            mutationTournamentSelectionNewGA();
            swapMutationNewGA();
            evaluateFitnessScoreNewGA(MUTATED_OFFSPRING_STORAGE_PLACE, pq, mandatory_count);
            mutationElitismViolations();
            if (G_BEST[CHROMOSOME_SIZE_NEW_GA] > MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA]) {
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
                last_improvement_iter = i;
                /*printf("\nImprovement at iteration:%d", last_improvement_iter);
                printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
            }
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] == 0) {
                feasible = 1;
                if (G_BEST[CHROMOSOME_SIZE_NEW_GA] != 0)
                    memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));

                if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]) {
                    memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
                    last_improvement_iter = i;
                    /*printf("\nImprovement at iteration:%d", last_improvement_iter);
                    printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
                }
            }
        }

        // Checking for CONVERGENCE_STOPPING_CRITERION
        /*if (G_BEST[CHROMOSOME_SIZE_NEW_GA] == g_best)
            same_fitness_iter++;
        else {
            same_fitness_iter = 0;*/
        g_best = G_BEST[CHROMOSOME_SIZE_NEW_GA];
        // printf("\nIteration number: %d", i);
     //}
    }

    printf("\nViolations in G_BEST: %d", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
    //minimizeCost(pq);
    printf("\nCost of G_BEST: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);
    printf("Number of iterations: %d\n", i);
    printf("\nLast improvement from the population:%d", last_improvement);
    printf("\nLast improvement in the iteration:%d", last_improvement_iter);
}

//void applyGeneticAlgorithmNewGA(PriorityQueue* pq) {
//    int i, j;
//    unsigned int same_fitness_iter = 0; // Initialize to prevent undefined behavior
//    float p_c = 0.85;
//
//    generatePopulationNewGA();
//
//    // Evaluate initial population and find the best chromosome
//    for (i = 0; i < POPULATION_SIZE; ++i)
//        evaluateFitnessScoreNewGA(POPULATION[i], pq);
//
//    // Initialize G_BEST with the best chromosome in the initial population
//    int best_idx = 0;
//    for (i = 1; i < POPULATION_SIZE; ++i) {
//        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] < POPULATION[best_idx][CHROMOSOME_SIZE_NEW_GA] ||
//            (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] == POPULATION[best_idx][CHROMOSOME_SIZE_NEW_GA] &&
//                POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] < POPULATION[best_idx][CHROMOSOME_SIZE_NEW_GA + 1])) {
//            best_idx = i;
//        }
//    }
//    memcpy(G_BEST, POPULATION[best_idx], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
//
//    int prev_best_violations = G_BEST[CHROMOSOME_SIZE_NEW_GA];
//    int prev_best_cost = G_BEST[CHROMOSOME_SIZE_NEW_GA + 1];
//
//    // Main GA loop
//    for (i = 0; i < NUM_ITER; ++i) { // Fixed off-by-one error
//        // Crossover or mutation
//        if ((rand() / (float)RAND_MAX) <= p_c) {
//            crossoverTournamentSelectionNewGA();
//            orderCrossoverNewGA();
//
//            for (j = 0; j < 2; ++j) {
//                evaluateFitnessScoreNewGA(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq);
//                // Update G_BEST if offspring is better
//                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA] < G_BEST[CHROMOSOME_SIZE_NEW_GA] ||
//                    (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA] == G_BEST[CHROMOSOME_SIZE_NEW_GA] &&
//                        CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1])) {
//                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
//                }
//            }
//            crossoverElitismViolations();
//        }
//        else {
//            mutationTournamentSelectionNewGA();
//            swapMutationNewGA();
//            evaluateFitnessScoreNewGA(MUTATED_OFFSPRING_STORAGE_PLACE, pq);
//
//            // Update G_BEST if mutated offspring is better
//            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] < G_BEST[CHROMOSOME_SIZE_NEW_GA] ||
//                (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] == G_BEST[CHROMOSOME_SIZE_NEW_GA] &&
//                    MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1])) {
//                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
//            }
//            mutationElitismViolations();
//        }
//
//        // Check for convergence
//        if (G_BEST[CHROMOSOME_SIZE_NEW_GA] == prev_best_violations &&
//            G_BEST[CHROMOSOME_SIZE_NEW_GA + 1] == prev_best_cost) {
//            same_fitness_iter++;
//          //  if (same_fitness_iter >= CONVERGENCE_STOPPING_CRITERION) break;
//        }
//        else {
//            same_fitness_iter = 0;
//            prev_best_violations = G_BEST[CHROMOSOME_SIZE_NEW_GA];
//            prev_best_cost = G_BEST[CHROMOSOME_SIZE_NEW_GA + 1];
//        }
//
//        // Early exit if perfect solution found
//        if (G_BEST[CHROMOSOME_SIZE_NEW_GA] == 0) break;
//    }
//
//    // Final check and output
//    printf("\nViolations in G_BEST: %d", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
//   /* if (G_BEST[CHROMOSOME_SIZE_NEW_GA] != 0) {
//        puts("\nCouldn't find a chromosome with 0 violations.\nExiting...");
//        exit(EXIT_FAILURE);
//    }*/
//
//    minimizeCost(pq);
//    printf("\nCost of G_BEST: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);
//    printf("Number of iterations: %d", i);
//}

//void applyGeneticAlgorithmNewGA(PriorityQueue* pq)
//{
//    int i, j, best_fitness = INT_MAX, g_best = INT_MAX, best_fitness_mandatory = 0, minimized_cost = 0, feasible = 0,flag = 0;
//    unsigned int same_fitness_iter = 0;
//    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
//    float p_m = 0.15;
//    int last_improvement = 0;
//    int last_improvement_iter = 0;
//
//    generatePopulationNewGA();
//    evaluateFitnessScoreNewGA(POPULATION[0], pq, mandatory_count);
//    /*memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));*/
//    for (i = 0; i < POPULATION_SIZE; ++i) {
//        evaluateFitnessScoreNewGA(POPULATION[i], pq, mandatory_count);
//        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] != -1 && !flag) {
//            memcpy(G_BEST, POPULATION[i], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
//            flag = 1;
//        }
//        if (G_BEST[CHROMOSOME_SIZE_NEW_GA] > POPULATION[i][CHROMOSOME_SIZE_NEW_GA]) {
//            memcpy(G_BEST, POPULATION[i], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
//            last_improvement = i;
//            //break;
//           /* printf("\nImprovement in population:%d", last_improvement);
//            printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
//        }
//        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] == 0 ) {
//            feasible = 1;  // Mark that at least one feasible solution has been found
//            //if(G_BEST[])
//            if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]) {
//                memcpy(G_BEST, POPULATION[i], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
//                last_improvement = i;
//                /*printf("\nImprovement in population:%d", last_improvement);
//                printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
//            }
//        }
//    }
//
//    for (i = 0; i < NUM_ITER; ++i) {
//        //if (G_BEST[CHROMOSOME_SIZE_NEW_GA] == 0) break;
//         if ((rand() / (float)RAND_MAX) <= p_c) {
//            crossoverTournamentSelectionNewGA();
//            orderCrossoverNewGA();
//            for (j = 0; j < 2; ++j)
//                evaluateFitnessScoreNewGA(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq, mandatory_count);
//            crossoverElitismViolations();
//            for (j = 0; j < 2; ++j) {
//                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA] == 0) {
//                   feasible = 1;
//                   if (G_BEST[CHROMOSOME_SIZE_NEW_GA] > CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA])
//                   {
//                       memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
//                       last_improvement_iter = i;
//                       //break;
//                      /* printf("\nImprovement at iteration:%d", last_improvement_iter);
//                       printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
//                   }
//                       if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]) {
//                        memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
//                        last_improvement_iter = i;
//                        //break;
//                       /* printf("\nImprovement at iteration:%d", last_improvement_iter);
//                        printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
//                    }
//                }
//            }
//        }
//        else  {
//            mutationTournamentSelectionNewGA();
//            swapMutationNewGA();
//            evaluateFitnessScoreNewGA(MUTATED_OFFSPRING_STORAGE_PLACE, pq, mandatory_count);
//            mutationElitismViolations();
//            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] == 0) {
//                feasible = 1;
//                if (G_BEST[CHROMOSOME_SIZE_NEW_GA] > MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA]) {
//                    memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
//                    last_improvement_iter = i;
//                    /*printf("\nImprovement at iteration:%d", last_improvement_iter);
//                    printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
//                }
//                if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]) {
//                    memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
//                    last_improvement_iter = i;
//                    /*printf("\nImprovement at iteration:%d", last_improvement_iter);
//                    printf("\nCost of the current_solution:%d", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);*/
//                }
//            }
//        }
//
//        // Checking for CONVERGENCE_STOPPING_CRITERION
//        /*if (G_BEST[CHROMOSOME_SIZE_NEW_GA] == g_best)
//            same_fitness_iter++;
//        else {
//            same_fitness_iter = 0;*/
//        g_best = G_BEST[CHROMOSOME_SIZE_NEW_GA];
//        // printf("\nIteration number: %d", i);
//     //}
//    }
//
//    printf("\nViolations in G_BEST: %d", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
//    //minimizeCost(pq);
//    printf("\nCost of G_BEST: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);
//    printf("Number of iterations: %d\n", i);
//    printf("\nLast improvement from the population:%d", last_improvement);
//    printf("\nLast improvement in the iteration:%d", last_improvement_iter);
//}

void evaluateFitnessScoreNewGA(int* chromosome, PriorityQueue* pq, int size)
{   // fitness score is the number of mandatory patients who were admitted in the scheduling period.
    // The best fitness score is CHROMOSOME_SIZE_NEW_GA
    reset_valuesNewGA();
    chromosome[size] = admitPatientsNewGA(&room_gender_map, pq, chromosome, size);
    //if (chromosome[CHROMOSOME_SIZE_NEW_GA] == 0) getchar();
    if (chromosome[size] == -1) return;
    /*if (chromosome[size] == 0) {
        printf("violation:");
        getchar();
    }*/
    chromosome[size] = findViolations();
    chromosome[size + 1] = findCost();
    //reset_valuesNewGA();

    //empty_pq(pq);
}

void minimizeCost(PriorityQueue* pq)
{
    int i, j, r1, r2;
    int* TEMP_G_BEST;
    TEMP_G_BEST = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
    memcpy(TEMP_G_BEST, G_BEST, (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
    TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA] = -1;
    TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA + 1] = -1;

    for (i = 0; i < NUM_COST_ITER && G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]; ++i) {
        /*
        1. find 2 random numbers from [0, chromosome size)
        2. swap the associated genes in the G_BEST chromosome
        3. evaluate the new fitness ensuring that #viiolations is 0 and the new cost is less than the previous one.
        4. if there exists such a chromosome - then replace it with the G_BEST
        5. repeat the process until the cost converges to 0 (and violations too) OR NUM_COST_ITER gets over.
        */

        do {
            r1 = rand() % (CHROMOSOME_SIZE_NEW_GA);
            r2 = rand() % (CHROMOSOME_SIZE_NEW_GA);
        } while (r1 == r2);

        TEMP_G_BEST[r1] += TEMP_G_BEST[r2];
        TEMP_G_BEST[r2] = TEMP_G_BEST[r1] - TEMP_G_BEST[r2];
        TEMP_G_BEST[r1] -= TEMP_G_BEST[r2];

        evaluateFitnessScoreNewGA(TEMP_G_BEST, pq, mandatory_count);
        if (TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA]) {
            memcpy(TEMP_G_BEST, G_BEST, (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
            continue;
        }
        //replace G_BEST if the cost of TEMP_G_BEST is less than that of the G_BEST
        if (TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA + 1] < G_BEST[CHROMOSOME_SIZE_NEW_GA + 1])
            memcpy(G_BEST, TEMP_G_BEST, (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
    }
    /*
    We can parallelize this process by first finding 4 chromosomes that have 0 violations and then putting each of them on one thread/core and
    running this loop for each of them.
    */
}

// To be implemented-----------------------------------------------------------------------------------------------------------------------------
int findViolations(void) {
    // add validator code here
    int cost = 0;
    for (int i = 0; i < num_patients; i++) {
        if (patients[i].admission_day == -1 && patients[i].mandatory) cost++;
    }
    return cost;
}

int findCost(void) {
    // add validator code here
    int final_cost = 0;
    final_cost += patient_delay();
    final_cost += elective_unscheduled_patients();
    final_cost += open_operating_theatre();
    //final_cost += room_age_mix(); // not written correctly
    //final_cost += skill_level_mix();
    return final_cost;
}

//……………PATIENT DELAY………..//
int patient_delay() {
    int cost = 0;
    for (int p = 0; p < num_patients; p++) {
        if (patients[p].admission_day != -1 && patients[p].admission_day > patients[p].surgery_release_day) {
            cost += patients[p].admission_day - patients[p].surgery_release_day;
        }
    }
    return cost;
}

//…………….OPTIONAL UNSCHEDULED…………..//
int elective_unscheduled_patients() {
    int cost = 0;
    for (int i = 0; i < num_patients; i++) {
        if (patients[i].admission_day == -1 && !patients[i].mandatory) {
            cost++;
        }
    }
    return cost;
}

//…………..OPEN OTS………………….//
int open_operating_theatre() {
    int cost = 0;
    for (int t = 0; t < num_ots; t++) {
        for (int d = 0; d < days; d++) {
            if (ot[t].max_ot_time[d] != ot[t].time_left[d]) {
                cost++;
            }
        }
    }
    return cost;
}

//……………ROOM AGE MIX…………………..//
int room_age_mix() {
    int cost = 0;

    for (int i = 0; i < num_rooms; i++) {
        for (int j = 0; j < days; j++) {
            if (size_of_room_schedule[i][j]) {
                int min_age = 1000, max_age = -1;

                for (int m = 0; m < size_of_room_schedule[i][j]; m++) {
                    char* entry = room_schedule[i][j][m];

                    int id = str2int(entry);
                    char* age = (entry[0] == 'p') ? patients[id].age_group : occupants[id].age;
                    //'''''''''''''''''''''''''''''''''''''''''''''''''change'''''''''''''''''''''''''''''''''''''''''''''''''''''''
                    if (age < min_age) min_age = age;
                    if (age > max_age) max_age = age;
                }
                cost += (max_age - min_age);
            }
        }
    }
    return cost;
}

int skill_level_mix() {
    int cost = 0, id, n, day, s1;
    char* p;

    for (int i = 0; i < num_rooms; i++) {
        for (int s = 0; s < 3 * days; s++) {
            day = s / 3;
            n = room_shift_nurse[i][s];

            for (int j = 0; j < size_of_room_schedule[i][day]; j++) {
                p = room_schedule[i][day][j];
                id = str2int(p);

                if (p[0] == 'p') {  // Check if it's a patient
                    s1 = s - patients[id].admission_day * 3;
                    if (patients[id].skill_level_required[s1] > nurses[n].skill_level) {
                        cost += patients[id].skill_level_required[s1] - nurses[n].skill_level;
                    }
                }
                else {
                    if (occupants[id].skill_level_required[s] > nurses[n].skill_level) {
                        cost += occupants[id].skill_level_required[s] - nurses[n].skill_level;
                    }
                }
            }
        }
    }
    return cost;
}



int excessiveNurseload() {
    int n, s, r, load, shift_var, cost = 0;
    for (n = 0; n < num_nurses; n++) {
        for (s = 0; s < nurses[n].num_shifts; s++) {
            int day = nurses[n].shift[s].day;
            int shift_time = nurses[n].shift[s].shift_time;
            shift_var = 3 * day + shift_time;
            for (r = 0; r < nurses[n].shift[s].num_rooms; r++) {
                load = rooms_requirement[nurses[n].shift[s].rooms[r]][shift_var].load_sum;
                if (load > nurses[n].shift[s].load_left)
                    cost += load - nurses[n].shift[s].load_left;
            }
        }
    }
    return cost;
}

int coc() {
    int cost = 0, count;
    for (int o = 0; o < num_occupants; o++)
    {
        count = countdistinctnurses_occupants(o);
        if (count > 0)
        {
            cost += count;
        }
    }
    for (int p = 0; p < num_patients; p++)
    {
        if (patients[p].admission_day != -1)
        {
            count = countdistinctnurses(p);
            if (count > 0)
            {
                cost += count;
            }
        }
    }
    return cost;

}

int countdistinctnurses(int p) {
    int* tag = (int*)calloc(num_nurses, sizeof(int));
    int i, n, count = 0;
    for (i = 0; i < patients[p].num_nurses_allotted; i++) {
        n = patients[p].nurses_allotted[i];
        if (n != -1 && !tag[n]) {
            tag[n] = 1;
            count++;
        }
    }
    return count;
}

int countdistinctnurses_occupants(int o) {
    int* tag = (int*)calloc(num_nurses, sizeof(int));
    int i, n, count = 0;
    for (i = 0; i < occupants[o].num_nurses_alloted; i++) {
        n = occupants[o].nurses_alloted[i];
        if (n != -1 && !tag[n]) {
            tag[n] = 1;
            count++;
        }
    }
    return count;
}


void crossoverTournamentSelectionNewGA(void)
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
    f11 = POPULATION[r11][CHROMOSOME_SIZE_NEW_GA];
    f12 = POPULATION[r12][CHROMOSOME_SIZE_NEW_GA];
    f13 = POPULATION[r13][CHROMOSOME_SIZE_NEW_GA];
    best_fitness = f11;
    best_fitness_idx1 = r11;

    if (f12 < best_fitness) {
        if (f13 < f12) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }
        else {
            best_fitness = f12;
            best_fitness_idx1 = r12;
        }
    }
    else
        if (f13 < best_fitness) {
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
    f21 = POPULATION[r21][CHROMOSOME_SIZE_NEW_GA];
    f22 = POPULATION[r22][CHROMOSOME_SIZE_NEW_GA];
    f23 = POPULATION[r23][CHROMOSOME_SIZE_NEW_GA];
    best_fitness = f21;
    best_fitness_idx2 = r21;

    if (f22 < best_fitness) {
        if (f23 < f22) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
        else {
            best_fitness = f22;
            best_fitness_idx2 = r22;
        }
    }
    else
        if (f23 < best_fitness) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[0], POPULATION[best_fitness_idx1], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[1], POPULATION[best_fitness_idx2], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
}

void mutationTournamentSelectionNewGA(void)
{   // select 2 parents using Tournament Selection method
    int r1, r2, r3, f1, f2, f3;
    int best_fitness, best_fitness_idx;

    // select first parent
    do {
        r1 = rand() % (POPULATION_SIZE);
        r2 = rand() % (POPULATION_SIZE);
        r3 = rand() % (POPULATION_SIZE);
    } while (r1 == r2 || r2 == r3 || r1 == r3);

    // select a chromosome with the best fitness
    f1 = POPULATION[r1][CHROMOSOME_SIZE_NEW_GA];
    f2 = POPULATION[r2][CHROMOSOME_SIZE_NEW_GA];
    f3 = POPULATION[r3][CHROMOSOME_SIZE_NEW_GA];
    best_fitness = f1;
    best_fitness_idx = r1;

    if (f2 < best_fitness) {
        if (f3 < f2) {
            best_fitness = f3;
            best_fitness_idx = r3;
        }
        else {
            best_fitness = f2;
            best_fitness_idx = r2;
        }
    }
    else
        if (f3 < best_fitness) {
            best_fitness = f3;
            best_fitness_idx = r3;
        }

    memcpy(MUTATE_PARENT_STORAGE_PLACE, POPULATION[best_fitness_idx], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
}

void crossoverElitismViolations(void)
{
    int i, worst_fitness_chromosome_index = 0, second_worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA])
            worst_fitness_chromosome_index = i;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA] > POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA])
        memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
    else {
        if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA])
            memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
        return;
    }
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA] <= POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA])
        memcpy(POPULATION[second_worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
}

void mutationElitismViolations(void)
{
    int i, worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA])
        memcpy(POPULATION[worst_fitness_chromosome_index], MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
}

void crossoverElitismCost(void)
{
    int i, worst_fitness_chromosome_index = 0, second_worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
            worst_fitness_chromosome_index = i;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] > POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA + 1] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
        memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
    else {
        if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA + 1] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
            memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
        return;
    }
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA + 1] <= POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
        memcpy(POPULATION[second_worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
}

void mutationElitismCost(void)
{
    int i, worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA + 1] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA + 1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA + 1])
        memcpy(POPULATION[worst_fitness_chromosome_index], MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
}

void generateNewChromosomeNewGA(int chromo_num)
{
    int j, r1, r2;

    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
    for (j = 0; j < size; ++j)
        POPULATION[chromo_num][j] = POPULATION[chromo_num - 1][j];

    for (j = 0; j < size; ++j) {
        do {
            r1 = rand() % (size);
            r2 = rand() % (size);
        } while (r1 == r2);

        swapGenes(chromo_num, r1, r2);
    }
}

void generatePopulationNewGA(void)
{
    int i, j, k;
    for (j = 0; j < mandatory_count; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;
    /* for (i = j; i < CHROMOSOME_SIZE_NEW_GA; ++i)
         POPULATION[0][i] = optional_patients[i - j]->id;*/
    for (i = 1; i < POPULATION_SIZE; ++i)
        generateNewChromosomeNewGA(i);
}


void initDataStructuresNewGA(int size)
{
    int i;

    // Allocate memory for single arrays
    MUTATED_OFFSPRING_STORAGE_PLACE = (int*)calloc(size + 2, sizeof(int));
    ASSERT(MUTATED_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for MUTATED_OFFSPRING_STORAGE_PLACE");

    MUTATE_PARENT_STORAGE_PLACE = (int*)calloc(size + 2, sizeof(int));
    ASSERT(MUTATE_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for MUTATE_PARENT_STORAGE_PLACE");

    G_BEST = (int*)calloc(size + 2, sizeof(int));
    ASSERT(G_BEST, "Dynamic Memory Allocation Error for G_BEST");

    // Allocate memory for crossover offspring storage
    CROSSOVER_OFFSPRING_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(CROSSOVER_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        CROSSOVER_OFFSPRING_STORAGE_PLACE[i] = (int*)calloc(size + 2, sizeof(int));
        ASSERT(CROSSOVER_OFFSPRING_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE[i]");
    }

    // Allocate memory for crossover parent storage
    CROSSOVER_PARENT_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(CROSSOVER_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        CROSSOVER_PARENT_STORAGE_PLACE[i] = (int*)calloc(size + 2, sizeof(int));
        ASSERT(CROSSOVER_PARENT_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE[i]");
    }

    POPULATION = (int**)calloc(POPULATION_SIZE, sizeof(int*));
    ASSERT(POPULATION, "Dynamic Memory Allocation Error for POPULATION");

    for (i = 0; i < POPULATION_SIZE; ++i) {
        POPULATION[i] = (int*)calloc(size + 2, sizeof(int));
        ASSERT(POPULATION[i], "Dynamic Memory Allocation Error for a CHROMOSOME");

        POPULATION[i][size] = -1;
        POPULATION[i][size + 1] = -1;
    }
}

void append_optionalsNewGA() {
    // Attempt to reallocate memory
    int a = G_BEST[CHROMOSOME_SIZE_NEW_GA];
    int b = G_BEST[CHROMOSOME_SIZE_NEW_GA + 1];
    G_BEST = (int*)realloc(G_BEST, (mandatory_count) * sizeof(int));
    G_BEST = (int*)realloc(G_BEST, (num_patients + 2) * sizeof(int));
    int* optionals = (int*)calloc(optional_count, sizeof(int));
    if (!optionals) {
        perror("Memory allocation failed for optionals");
        exit(EXIT_FAILURE);
    }

    // Store optional patient IDs in the array
    for (int i = 0; i < optional_count; i++) {
        optionals[i] = optional_patients[i]->id;
    }
    //if (!temp) {
    //    printf("Memory allocation failed!\n");
    //    return G_BEST;  // Return original pointer if realloc fails
    //}
    //G_BEST = temp;

    // Preserve existing values
    G_BEST[num_patients] = a;     // Copy feasibility score
    G_BEST[num_patients + 1] = b;  // Copy cost

    // Append optional patient IDs at the end
    for (int i = 0; i < optional_count; i++) {
        G_BEST[i + mandatory_count] = optionals[i];  // Ensure correct indexing
    }

    //return G_BEST;  // Return updated pointer
}

void swapMutationoptionals(int* chromosome) {
    if (!chromosome) return;

    int pos1, pos2;

    // Ensuring we only swap within the valid gene range
    do {
        pos1 = rand() % num_patients;
        pos2 = rand() % num_patients;
    } while (pos1 == pos2);  // Avoid swapping the same position

    // Swap values
    int temp = chromosome[pos1];
    chromosome[pos1] = chromosome[pos2];
    chromosome[pos2] = temp;
    last_swap_pos1 = pos1;
    last_swap_pos2 = pos2;
    //printf("\nPerformed swap mutation: Swapped positions %d and %d.\n", pos1, pos2);
}

void reverseSwap(int* chromosome) {
    if (!chromosome || last_swap_pos1 == -1 || last_swap_pos2 == -1) return;

    // Swap back to reverse the mutation
    int temp = chromosome[last_swap_pos1];
    chromosome[last_swap_pos1] = chromosome[last_swap_pos2];
    chromosome[last_swap_pos2] = temp;

    // printf("\nReversed swap mutation: Swapped positions %d and %d back.\n", last_swap_pos1, last_swap_pos2);

     // Reset last swap tracking
    last_swap_pos1 = -1;
    last_swap_pos2 = -1;
}

void ApplyHillClimbing(int* G_BEST, PriorityQueue* pq) {
    int last_improv = 0, no_of_improv = 0;
    int i = 0;
    int* temp_solution = (int*)calloc((num_patients + 2), sizeof(int));

    if (!temp_solution) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(temp_solution, G_BEST, (num_patients + 2) * sizeof(int));

    printf("\nPerforming Genetic Algorithm with Optional Patients...");

    // Mutation loop: keep swapping until a feasible solution is found
    while (i < 100000) {
        /*int* temp_copy = (int*)calloc((num_patients + 2), sizeof(int));
        if (!temp_copy) {
            printf("Memory allocation failed.\n");
            free(temp_solution);
            exit(EXIT_FAILURE);
        }*/
        // memcpy(temp_solution, G_BEST, (num_patients + 2) * sizeof(int));

         // Apply swap mutation on a copy to test feasibility
        swapMutationoptionals(temp_solution);

        // Evaluate fitness
        evaluateFitnessScoreNewGA(temp_solution, pq, num_patients);
        // if (temp_copy[num_patients] == 0) getchar();
         // If the new solution is feasible, accept the mutation
        if (temp_solution[num_patients] == 0) {
            //memcpy(temp_solution, temp_copy, (num_patients + 2) * sizeof(int));

            // Check for improvement and update G_BEST
           // if (temp_solution[num_patients + 1] < G_BEST[num_patients + 1]) {
            memcpy(G_BEST, temp_solution, (num_patients + 2) * sizeof(int));
            last_improv = i;
            no_of_improv++;
            // }
        }
        else {
            reverseSwap(temp_solution);
        }

        //free(temp_copy);
        i++;
    }

    printf("\nG_BEST : \n");
    for (int i = 0; i < optional_count + mandatory_count + 2; i++) {
        printf("%d\t", G_BEST[i]);
    }
    printf("\nFinal feasible G_BEST after swap mutation applied.");
    printf("\nLast improvement from hill climbing: %d", last_improv);
    printf("\nNumber of improvements: %d", no_of_improv);
    printf("\nViolations: %d", G_BEST[num_patients]);

    free(temp_solution);
}

//#include <windows.h>
//
//void* ApplyHillClimbingThread(void* arg) {
//    ThreadData* data = (ThreadData*)arg;
//    int* local_solution = (int*)calloc((num_patients + 2), sizeof(int));
//
//    if (!local_solution) {
//        printf("Memory allocation failed in thread %d.\n", data->thread_id);
//        pthread_exit(NULL);
//    }
//
//    memcpy(local_solution, data->G_BEST, (num_patients + 2) * sizeof(int));
//
//    int i = 0;
//    while (!stop_threads) {
//        swapMutationoptionals(local_solution);
//        evaluateFitnessScoreNewGA(local_solution, data->pq, num_patients);
//
//        // Extract the fitness cost stored at index num_patients + 1
//        int fitness = local_solution[num_patients + 1];
//
//        // Check if solution is feasible and update the global best solution
//        if (local_solution[num_patients] == 0) {
//            pthread_mutex_lock(&lock);
//            if (fitness < global_best_fitness) {
//                global_best_fitness = fitness;
//                memcpy(global_best_solution, local_solution, (num_patients + 2) * sizeof(int));
//                printf("[Thread %d] New best solution found with cost: %d\n", data->thread_id, global_best_fitness);
//            }
//            pthread_mutex_unlock(&lock);
//        }
//        else {
//            reverseSwap(local_solution);
//        }
//
//        i++;
//        if (i % 1000 == 0) {
//            Sleep(CHECK_INTERVAL * 1000); // Convert seconds to microseconds
//        }
//    }
//
//    free(local_solution);
//    pthread_exit(NULL);
//}
//
//void ApplyHillClimbingMultiThreaded(PriorityQueue* pq) {
//    pthread_t threads[NUM_THREADS];
//    ThreadData thread_data[NUM_THREADS];
//
//    pthread_mutex_init(&lock, NULL);
//    global_best_solution = (int*)malloc((num_patients + 2) * sizeof(int));
//    memcpy(global_best_solution, G_BEST, (num_patients + 2) * sizeof(int));
//
//    // Create threads
//    for (int i = 0; i < NUM_THREADS; i++) {
//        thread_data[i].G_BEST = G_BEST;
//        thread_data[i].pq = pq;
//        thread_data[i].thread_id = i;
//        pthread_create(&threads[i], NULL, ApplyHillClimbingThread, (void*)&thread_data[i]);
//    }
//
//    int elapsed_time = 0;
//    while (elapsed_time < 600) { // Run for 10 minutes
//        Sleep(CHECK_INTERVAL * 1000);
//        elapsed_time += CHECK_INTERVAL;
//
//        // Lock before updating all threads
//        pthread_mutex_lock(&lock);
//        printf("\n[INFO] 2 minutes passed, updating all threads with best solution (Cost: %d).\n", global_best_fitness);
//        for (int i = 0; i < NUM_THREADS; i++) {
//            memcpy(thread_data[i].G_BEST, global_best_solution, (num_patients + 2) * sizeof(int));
//        }
//        pthread_mutex_unlock(&lock);
//    }
//
//    stop_threads = 1;
//
//    for (int i = 0; i < NUM_THREADS; i++) {
//        pthread_join(threads[i], NULL);
//    }
//
//    memcpy(G_BEST, global_best_solution, (num_patients + 2) * sizeof(int));
//
//    free(global_best_solution);
//    pthread_mutex_destroy(&lock);
//}

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

//------------------------------------------------------BELOW: NURSE GA FUNCTOINS---------------------------------------------------------------------

int NURSE_POPULATION_SIZE = 1000, NURSE_NUM_ITER = 100000 , NURSE_CONVERGENCE_STOPPING_CRITERION;
int** NURSE_POPULATION, * N_G_BEST;
int CHROMOSOME_SIZE_NURSE_GA = 0;
int** NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE, ** NURSE_CROSSOVER_PARENT_STORAGE_PLACE;
int* NURSE_MUTATED_OFFSPRING_STORAGE_PLACE, * NURSE_MUTATE_PARENT_STORAGE_PLACE, * nurse_chromosome;

//--------------------------------------------------------------BELOW: NURSE GA FUNCTION PROTOTYPES-------------------------------------------------

void crossoverTournamentSelectionNurseGA(void);
void mutationTournamentSelectionNurseGA(void);
void NurseCrossoverElitismCost(void);
void NurseMutationElitismCost(void);
void generateNewChromosomeNurseGA(int chromo_num);
void generatePopulationNurseGA(void);
void initDataStructuresNurseGA(void);
void freeDataStructuresNurseGA(void);
void swapGenesNurseGA(int chromo_num, int gene1, int gene2);
void applyNursesGA(void);
void evaluateFitnessNurseGA(int* nurse_chromosome);
void resetValuesNurseGA(void);
void orderCrossoverNurseGA(void);
void swapMutationNurseGA(void);
void printPopulationNurseGA(void);
int evaluateCostNurseGA(void);
int nurseAllocationGA(int*);

int countdistinctnurses_occupants(int o);
int countdistinctnurses(int p);
int coc();
int excessiveNurseload();

//--------------------------------------------------------------ABOVE: NURSE GA FUNCTION PROTOTYPES-------------------------------------------------

void crossoverTournamentSelectionNurseGA(void)
{   // select 2 parents using Tournament Selection method
    int r11, r12, r13, r21, r22, r23, f11, f12, f13, f21, f22, f23;
    int best_fitness, best_fitness_idx1, best_fitness_idx2;

    // select first parent
    do {
        r11 = rand() % (NURSE_POPULATION_SIZE);
        r12 = rand() % (NURSE_POPULATION_SIZE);
        r13 = rand() % (NURSE_POPULATION_SIZE);
    } while (r11 == r12 || r12 == r13 || r11 == r13);

    // select the chromosome1 with the best fitness
    f11 = NURSE_POPULATION[r11][CHROMOSOME_SIZE_NURSE_GA];
    f12 = NURSE_POPULATION[r12][CHROMOSOME_SIZE_NURSE_GA];
    f13 = NURSE_POPULATION[r13][CHROMOSOME_SIZE_NURSE_GA];
    best_fitness = f11;
    best_fitness_idx1 = r11;

    if (f12 < best_fitness) {
        if (f13 < f12) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }
        else {
            best_fitness = f12;
            best_fitness_idx1 = r12;
        }
    }
    else
        if (f13 < best_fitness) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }

    // select second parent
    do {
        r21 = rand() % (NURSE_POPULATION_SIZE);
        r22 = rand() % (NURSE_POPULATION_SIZE);
        r23 = rand() % (NURSE_POPULATION_SIZE);
    } while (r21 == r22 || r22 == r23 || r21 == r23 ||
        r11 == r21 || r11 == r22 || r11 == r23 ||
        r12 == r21 || r12 == r22 || r12 == r23 ||
        r13 == r21 || r13 == r22 || r13 == r23);

    // select the chromosome2 with the best fitness
    f21 = NURSE_POPULATION[r21][CHROMOSOME_SIZE_NURSE_GA];
    f22 = NURSE_POPULATION[r22][CHROMOSOME_SIZE_NURSE_GA];
    f23 = NURSE_POPULATION[r23][CHROMOSOME_SIZE_NURSE_GA];
    best_fitness = f21;
    best_fitness_idx2 = r21;

    if (f22 < best_fitness) {
        if (f23 < f22) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
        else {
            best_fitness = f22;
            best_fitness_idx2 = r22;
        }
    }
    else
        if (f23 < best_fitness) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
    memcpy(NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0], NURSE_POPULATION[best_fitness_idx1], (CHROMOSOME_SIZE_NURSE_GA + 1) * sizeof(int));
    memcpy(NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1], NURSE_POPULATION[best_fitness_idx2], (CHROMOSOME_SIZE_NURSE_GA + 1) * sizeof(int));
}

void mutationTournamentSelectionNurseGA(void)
{   // select 2 parents using Tournament Selection method
    int r1, r2, r3, f1, f2, f3;
    int best_fitness, best_fitness_idx;

    // select first parent
    do {
        r1 = rand() % (NURSE_POPULATION_SIZE);
        r2 = rand() % (NURSE_POPULATION_SIZE);
        r3 = rand() % (NURSE_POPULATION_SIZE);
    } while (r1 == r2 || r2 == r3 || r1 == r3);

    // select a chromosome with the best fitness
    f1 = NURSE_POPULATION[r1][CHROMOSOME_SIZE_NURSE_GA];
    f2 = NURSE_POPULATION[r2][CHROMOSOME_SIZE_NURSE_GA];
    f3 = NURSE_POPULATION[r3][CHROMOSOME_SIZE_NURSE_GA];
    best_fitness = f1;
    best_fitness_idx = r1;

    if (f2 < best_fitness) {
        if (f3 < f2) {
            best_fitness = f3;
            best_fitness_idx = r3;
        }
        else {
            best_fitness = f2;
            best_fitness_idx = r2;
        }
    }
    else
        if (f3 < best_fitness) {
            best_fitness = f3;
            best_fitness_idx = r3;
        }

    memcpy(NURSE_MUTATE_PARENT_STORAGE_PLACE, NURSE_POPULATION[best_fitness_idx], (CHROMOSOME_SIZE_NURSE_GA + 1) * sizeof(int));
}

void NurseCrossoverElitismCost(void)
{
    int i, worst_fitness_chromosome_index = 0, second_worst_fitness_chromosome_index = 0;

    for (i = 1; i < NURSE_POPULATION_SIZE; ++i)
        if (NURSE_POPULATION[i][CHROMOSOME_SIZE_NURSE_GA] > NURSE_POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA])
            worst_fitness_chromosome_index = i;

    for (i = 1; i < NURSE_POPULATION_SIZE; ++i)
        if (NURSE_POPULATION[i][CHROMOSOME_SIZE_NURSE_GA] > NURSE_POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NURSE_GA] <= NURSE_POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA])
        memcpy(NURSE_POPULATION[worst_fitness_chromosome_index], NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE_NURSE_GA + 1));
    else {
        if (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NURSE_GA] <= NURSE_POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA])
            memcpy(NURSE_POPULATION[worst_fitness_chromosome_index], NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NURSE_GA + 1));
        return;
    }
    if (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NURSE_GA] <= NURSE_POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA])
        memcpy(NURSE_POPULATION[second_worst_fitness_chromosome_index], NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NURSE_GA + 1));
}

void NurseMutationElitismCost(void)
{
    int i, worst_fitness_chromosome_index = 0;

    for (i = 1; i < NURSE_POPULATION_SIZE; ++i)
        if (NURSE_POPULATION[i][CHROMOSOME_SIZE_NURSE_GA] > NURSE_POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NURSE_GA] <= NURSE_POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NURSE_GA])
        memcpy(NURSE_POPULATION[worst_fitness_chromosome_index], NURSE_MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NURSE_GA + 1));
}

void swapGenesNurseGA(int chromo_num, int gene1, int gene2)
{
    int temp = NURSE_POPULATION[chromo_num][gene1];
    NURSE_POPULATION[chromo_num][gene1] = NURSE_POPULATION[chromo_num][gene2];
    NURSE_POPULATION[chromo_num][gene2] = temp;
}

void applyNursesGA(void)
{
    int i, j, best_fitness, g_best;
    unsigned int same_fitness_iter;
    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
    generatePopulationNurseGA();
    //printPopulationNurseGA();

    for (i = 0; i < NURSE_POPULATION_SIZE; ++i) {
        resetValuesNurseGA();
        evaluateFitnessNurseGA(NURSE_POPULATION[i]);
    }
    //printf("%d\n", POPULATION[0]);
    //memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE + 1) * sizeof(int));
    for (int i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
        N_G_BEST[i] = NURSE_POPULATION[0][i];
    }
    g_best = N_G_BEST[CHROMOSOME_SIZE_NURSE_GA];
  
    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    for (i = 0; i <= NURSE_NUM_ITER; ++i) {
        if ((rand() / (float)RAND_MAX) <= p_c) {
            crossoverTournamentSelectionNurseGA();
            orderCrossoverNurseGA();
            /*printf("\nCrossover Offsprings: ");
            printf("\nOffspring 2:");
            for (int m = 0; m < CHROMOSOME_SIZE_NURSE_GA; m++) {
                printf("%d\t", NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][m]);
            }
            printf("\nOffspring 2:");
            for (int m = 0; m < CHROMOSOME_SIZE_NURSE_GA; m++) {
                printf("%d\t", NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m]);
            }*/
            // calculate the fitness of the 2 new offsprings
            for (j = 0; j < 2; ++j)
                evaluateFitnessNurseGA(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[j]);
            NurseCrossoverElitismCost();
            for (j = 0; j < 2; ++j)
                if (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NURSE_GA] > N_G_BEST[CHROMOSOME_SIZE_NURSE_GA])
                    memcpy(G_BEST, NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NURSE_GA + 1));
        }
        else {
            mutationTournamentSelectionNurseGA();
            /*printf("\nMutation Parent: ");
            for (int i = 0; i < CHROMOSOME_SIZE; i++) {
                printf("%d\t", MUTATE_PARENT_STORAGE_PLACE[i]);
            }*/
            swapMutationNurseGA();
            // calculate the fitness of the new offspring pq
             evaluateFitnessNurseGA(NURSE_MUTATED_OFFSPRING_STORAGE_PLACE);
            NurseMutationElitismCost();
            if (NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NURSE_GA] > N_G_BEST[CHROMOSOME_SIZE_NURSE_GA])
                memcpy(N_G_BEST, NURSE_MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NURSE_GA + 1));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        best_fitness = N_G_BEST[CHROMOSOME_SIZE_NURSE_GA];
        /*if (best_fitness == g_best)
            same_fitness_iter++;
        else
            same_fitness_iter = 0;*/
        g_best = N_G_BEST[CHROMOSOME_SIZE_NURSE_GA];
    }
    //puts("\nChecking whether the iterations were over OR the algorithm converged: ");
    printf("Number of iterations: %d", i);
    // either i will be NUM_ITER (10000) OR same_fitness_iter will be CONVERGENCE_STOPPING_CRITERION (100).
    // G_BEST is the best chromosome - use it for admitting patients.
}

/*.........................
1. room_requ - #rooms X #shifts
2. room_schedule - #room X #days X #patient (in that room + on that day)
3. dm_nurse_availibility - #shifts X #nurses (matrix of struct pointers)
.........................*/

int nurseAllocationGA(int* nurse_chromosome)
{
    int n_id, r_id, r_index, fitness = 0, nurse_shift, n_index, p_index, sh, s;
    bool** track_nurse_shift = (bool**)calloc(num_rooms, sizeof(bool*));
    for (int i = 0; i < num_rooms; i++) {
        track_nurse_shift[i] = (bool*)calloc(days * 3, sizeof(bool));
        if (!track_nurse_shift[i]) {
            perror("Memory allocation failed");
            return 1;
        }
    }
    //put_occupants();

    for (n_index = 0; n_index < CHROMOSOME_SIZE_NURSE_GA; ++n_index) {
        n_id = nurse_chromosome[n_index];
        for (s = 0; s < nurses[n_id].num_shifts; s++) {
            int day = nurses[n_id].shift[s].day;
            int shift_time = nurses[n_id].shift[s].shift_time;
            nurse_shift = day * 3 + shift_time;
            for (r_index = 0; r_index < num_rooms; ++r_index) {
                r_id = room[r_index].id;
                if (track_nurse_shift[r_index][nurse_shift]) continue;
                if (nurses[n_id].shift[s].load_left < rooms_requirement[r_index][nurse_shift].load_sum) continue;
                track_nurse_shift[r_index][nurse_shift] = true;

                // Allocate room to nurse's shift
                int* new_rooms = realloc(nurses[n_id].shift[s].rooms, (nurses[n_id].shift[s].num_rooms + 1) * sizeof(int));
                if (!new_rooms) { ASSERT(0, "Memory Allocation Error!"); }
                nurses[n_id].shift[s].rooms = new_rooms;
                nurses[n_id].shift[s].rooms[nurses[n_id].shift[s].num_rooms++] = r_id;

                // Allocate nurse to room
                int* new_nurses = realloc(room[r_index].nurses_alloted, (room[r_index].length_of_nurses_alloted + 1) * sizeof(int));
                if (!new_nurses) { ASSERT(0, "Memory Allocation Error!"); }
                room[r_index].nurses_alloted = new_nurses;
                room[r_index].nurses_alloted[room[r_index].length_of_nurses_alloted++] = n_id;

                nurses[n_id].shift[s].load_left -= rooms_requirement[r_index][nurse_shift].load_sum;

                // Update patients and occupants
                for (p_index = 0; p_index < size_of_room_schedule[r_index][day]; ++p_index) {
                    char* entry = room_schedule[r_index][day][p_index];
                    if (*entry == 'p') {
                        int id = str2int(entry + 1); // Skip 'p'
                        int* new_nurses_p = realloc(patients[id].nurses_allotted, (patients[id].num_nurses_allotted + 1) * sizeof(int));
                        if (!new_nurses_p) { ASSERT(0, "Memory Allocation Error!"); }
                        patients[id].nurses_allotted = new_nurses_p;
                        patients[id].nurses_allotted[patients[id].num_nurses_allotted++] = n_id;
                    }
                    else if (*entry == 'a') {
                        int occ = str2int(entry + 1); // Skip 'a'
                        int* new_nurses_occ = realloc(occupants[occ].nurses_alloted, (occupants[occ].num_nurses_alloted + 1) * sizeof(int));
                        if (!new_nurses_occ) { ASSERT(0, "Memory Allocation Error!"); }
                        occupants[occ].nurses_alloted = new_nurses_occ;
                        occupants[occ].nurses_alloted[occupants[occ].num_nurses_alloted++] = n_id;
                    }
                }
            }
        }
    }

    // Assign remaining shifts
    for (r_index = 0; r_index < num_rooms; ++r_index) {
        r_id = room[r_index].id;
        for (sh = 0; sh < days * 3; ++sh) {
            if (track_nurse_shift[r_index][sh]) continue;

            int most_suitable_nurse = -1;
            int most_suitable_shift = -1;
            int max_load = -1;

            for (n_index = 0; n_index < CHROMOSOME_SIZE_NURSE_GA; ++n_index) {
                n_id = nurse_chromosome[n_index];
                for (s = 0; s < nurses[n_id].num_shifts; s++) {
                    if ((nurses[n_id].shift[s].day * 3 + nurses[n_id].shift[s].shift_time) != sh) continue;
                    if (nurses[n_id].shift[s].load_left > max_load) {
                        max_load = nurses[n_id].shift[s].load_left;
                        most_suitable_nurse = n_id;
                        most_suitable_shift = s;
                    }
                }
            }

            if (most_suitable_nurse == -1) continue; // No suitable nurse found

            int day = sh / 3;
            int shift_time = sh % 3;
            if (rooms_requirement[r_index][sh].load_sum > nurses[most_suitable_nurse].shift[most_suitable_shift].load_left) continue;

            track_nurse_shift[r_index][sh] = true;

            // Allocate room to nurse's shift
            int* new_rooms = realloc(nurses[most_suitable_nurse].shift[most_suitable_shift].rooms, (nurses[most_suitable_nurse].shift[most_suitable_shift].num_rooms + 1) * sizeof(int));
            if (!new_rooms) { ASSERT(0, "Memory Allocation Error!"); }
            nurses[most_suitable_nurse].shift[most_suitable_shift].rooms = new_rooms;
            nurses[most_suitable_nurse].shift[most_suitable_shift].rooms[nurses[most_suitable_nurse].shift[most_suitable_shift].num_rooms++] = r_id;

            // Allocate nurse to room
            int* new_nurses = realloc(room[r_index].nurses_alloted, (room[r_index].length_of_nurses_alloted + 1) * sizeof(int));
            if (!new_nurses) { ASSERT(0, "Memory Allocation Error!"); }
            room[r_index].nurses_alloted = new_nurses;
            room[r_index].nurses_alloted[room[r_index].length_of_nurses_alloted++] = most_suitable_nurse;

            nurses[most_suitable_nurse].shift[most_suitable_shift].load_left -= rooms_requirement[r_index][sh].load_sum;

            // Update patients and occupants
            for (p_index = 0; p_index < size_of_room_schedule[r_index][day]; ++p_index) {
                char* entry = room_schedule[r_index][day][p_index];
                if (*entry == 'p') {
                    int id = str2int(entry + 1);
                    int* new_nurses_p = realloc(patients[id].nurses_allotted, (patients[id].num_nurses_allotted + 1) * sizeof(int));
                    if (!new_nurses_p) { ASSERT(0, "Memory Allocation Error!"); }
                    patients[id].nurses_allotted = new_nurses_p;
                    patients[id].nurses_allotted[patients[id].num_nurses_allotted++] = most_suitable_nurse;
                }
                else if (*entry == 'a') {
                    int occ = str2int(entry + 1);
                    int* new_nurses_occ = realloc(occupants[occ].nurses_alloted, (occupants[occ].num_nurses_alloted + 1) * sizeof(int));
                    if (!new_nurses_occ) { ASSERT(0, "Memory Allocation Error!"); }
                    occupants[occ].nurses_alloted = new_nurses_occ;
                    occupants[occ].nurses_alloted[occupants[occ].num_nurses_alloted++] = most_suitable_nurse;
                }
            }
        }
    }

    // Free track_nurse_shift
    for (int i = 0; i < num_rooms; i++) {
        free(track_nurse_shift[i]);
    }
    free(track_nurse_shift);

    return evaluateCostNurseGA();
}

void evaluateFitnessNurseGA(int* nurse_chromosome)
{
    resetValuesNurseGA();
    nurse_chromosome[CHROMOSOME_SIZE_NURSE_GA] = nurseAllocationGA(nurse_chromosome);
    resetValuesNurseGA();
}

void resetValuesNurseGA(void) {
    int i, j;
    for (i = 0; i < num_rooms; i++) {
        for (j = 0; j < room[i].length_of_nurses_alloted; ++j) {
            room[i].nurses_alloted[j] = -1;
        }
        room[i].length_of_nurses_alloted = 0;
    }


        for (i = 0; i < num_patients; i++){
            for (j = 0; j < patients[i].num_nurses_allotted; ++j) {
                patients[i].nurses_allotted[j] = -1;
            }
            patients[i].num_nurses_allotted = 0;
         }

    for (i = 0; i < num_occupants; i++) {
        for (j = 0; j < occupants[i].num_nurses_alloted; ++j) {
            occupants[i].nurses_alloted[j] = -1;
        }
        occupants[i].num_nurses_alloted = 0;
    }

    for (i = 0; i < num_nurses; i++) {
        for (j = 0; j < nurses[i].num_shifts; j++) {
            nurses[i].shift[j].load_left = nurses[i].shift[j].max_load;
            for(int k = 0 ; k < nurses[i].shift[j].num_rooms ; k++)
                 nurses[i].shift[j].rooms[k] = -1;
            nurses[i].shift[j].num_rooms = 0;
        }
    }


}

int evaluateCostNurseGA(void)
{
    int cost = 0;
    cost += excessiveNurseload();
    cost += coc();
    return cost;
}

extern int excessiveNurseload(); /*{
    int n, s, r, load, cost, shift_var, cost = 0;
    for (n = 0; n < num_nurses; n++) {
        for (s = 0; s < nurses[n].num_shifts; s++) {
            int day = nurses[n].shift[s].day;
            int shift_time = nurses[n].shift[s].shift_time;
            shift_var = 3 * day + shift_time;
            for (r = 0; r < nurses[n].shift[s].num_rooms; r++) {
                load = rooms_requirement[nurses[n].shift[s].rooms[r]][shift_var].load_sum;
                if (load > nurses[n].shift[s].load_left)
                    cost += load - nurses[n].shift[s].load_left;
            }
        }
    }
    return cost;
}*/

extern int coc(); /*{
    int cost = 0, count;
    for (int o = 0; o < num_occupants; o++)
    {
        count = countdistinctnurses_occupants(o);
        if (count > 0)
        {
            cost += count;
        }
    }
    for (int p = 0; p < num_patients; p++)
    {
        if (patients[p].admission_day != -1)
        {
            count = countdistinctnurses(p);
            if (count > 0)
            {
                cost += count;
            }
        }
    }
    return cost;

}*/

extern int countdistinctnurses(int p); /*{
    int* tag = (int*)calloc(num_nurses, sizeof(int));
    int i, n, count = 0;
    for (i = 0; i < patients[p].num_nurses_allotted; i++) {
        n = patients[p].nurses_allotted[i];
        if (n != -1 && !tag[n]) {
            tag[n] = 1;
            count++;
        }
    }
    return count;
}*/

extern int countdistinctnurses_occupants(int o); /*{
    int* tag = (int*)calloc(num_nurses, sizeof(int));
    int i, n, count = 0;
    for (i = 0; i < occupants[o].num_nurses_alloted; i++) {
        n = occupants[o].nurses_alloted[i];
        if (n != -1 && !tag[n]) {
            tag[n] = 1;
            count++;
        }
    }
    return count;
}*/

void orderCrossoverNurseGA(void) {
    int r1, r2, i, k, m, iterations;
    int max = 0;

    // Early exit if chromosome size is invalid
    if (CHROMOSOME_SIZE_NURSE_GA <= 0) return;

    // Calculate maximum gene ID across both parents
    for (int idx = 0; idx < CHROMOSOME_SIZE_NURSE_GA; idx++) {
        int gene0 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0][idx];
        int gene1 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][idx];
        if (gene0 > max) max = gene0;
        if (gene1 > max) max = gene1;
    }

    // Allocate visited arrays safely
    bool* visited1 = (bool*)calloc(max + 1, sizeof(bool));
    bool* visited2 = (bool*)calloc(max + 1, sizeof(bool));
    if (!visited1 || !visited2) {
        printf("Memory allocation failed!\n");
        free(visited1); free(visited2);
        return;
    }

    // Select valid crossover points
    do {
        r1 = rand() % CHROMOSOME_SIZE_NURSE_GA;
        r2 = rand() % CHROMOSOME_SIZE_NURSE_GA;
    } while (r1 == r2);
    if (r1 > r2) { int temp = r1; r1 = r2; r2 = temp; }

    // Initialize offspring with -1 (unassigned)
    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = -1;
        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = -1;
    }

    // Copy segment from parents to offspring
    for (i = r1; i <= r2; i++) {
        int gene0 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0][i];
        int gene1 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][i];

        // Copy to offspring and mark visited (safely)
        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = gene0;
        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = gene1;
        if (gene0 >= 0) visited1[gene0] = true;  // FIX: Check gene >= 0
        if (gene1 >= 0) visited2[gene1] = true;  // FIX: Check gene >= 0
    }

    // Fill remaining positions for offspring0 from parent1
    k = (r2 + 1) % CHROMOSOME_SIZE_NURSE_GA;
    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
        int index = (r2 + 1 + i) % CHROMOSOME_SIZE_NURSE_GA;
        int gene = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][index];
        // FIX: Check gene validity before accessing visited
        if (gene >= 0 && !visited1[gene]) {
            // Find next empty slot in offspring0
            if(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] != -1) {
                k = (k + 1) % CHROMOSOME_SIZE_NURSE_GA;
            }
            NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] = gene;
            visited1[gene] = true;  // FIX: Mark as visited
        }
    }

    // Fill remaining positions for offspring1 from parent0
    m = (r2 + 1) % CHROMOSOME_SIZE_NURSE_GA;
    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
        int index = (r2 + 1 + i) % CHROMOSOME_SIZE_NURSE_GA;
        int gene = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0][index];
        // FIX: Check gene validity before accessing visited
        if (gene >= 0 && !visited2[gene]) {
            iterations = 0;
            // Find next empty slot in offspring1
            if(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] != -1) {
                m = (m + 1) % CHROMOSOME_SIZE_NURSE_GA;
                iterations++;
            }
            NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] = gene;
            visited2[gene] = true;  // FIX: Mark as visited
        }
    }
    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
        int index = (r2 + 1 + i) % CHROMOSOME_SIZE_NURSE_GA;
        int gene = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][index];
        if (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] == -1) {
            if (gene >= 0 && !visited1[gene]) {
                // Find next empty slot in offspring0
                if (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] != -1) {
                    k = (k + 1) % CHROMOSOME_SIZE_NURSE_GA;
                }
                NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] = gene;
                visited1[gene] = true;  // FIX: Mark as visited
            }
        }
    }
    

    free(visited1);
    free(visited2);
}

//void orderCrossoverNurseGA(void) {
//    int r1, r2, i, k, m;
//    int max = 0;
//
//    // Calculate maximum gene value from both parents
//    for (int idx = 0; idx < CHROMOSOME_SIZE_NURSE_GA; idx++) {
//        int gene0 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0][idx];
//        int gene1 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][idx];
//        if (gene0 > max) max = gene0;
//        if (gene1 > max) max = gene1;
//    }
//
//    bool* visited1 = (bool*)calloc(max + 1, sizeof(bool));
//    bool* visited2 = (bool*)calloc(max + 1, sizeof(bool));
//
//    if (!visited1 || !visited2) {
//        printf("Memory allocation failed!\n");
//        exit(1);
//    }
//
//    // Select crossover points
//    do {
//        r1 = rand() % CHROMOSOME_SIZE_NURSE_GA;
//        r2 = rand() % CHROMOSOME_SIZE_NURSE_GA;
//    } while (r1 == r2);
//
//    if (r1 > r2) {
//        int temp = r1;
//        r1 = r2;
//        r2 = temp;
//    }
//
//    // Initialize offspring with -1
//    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
//        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = -1;
//        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = -1;
//    }
//
//    // Copy segments and mark visited genes (safely)
//    for (i = r1; i <= r2; i++) {
//        int gene0 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0][i];
//        int gene1 = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][i];
//
//        // Offspring0 gets Parent0's segment
//        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = gene0;
//        if (gene0 >= 0) visited1[gene0] = true;
//
//        // Offspring1 gets Parent1's segment
//        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = gene1;
//        if (gene1 >= 0) visited2[gene1] = true;
//    }
//
//    // Fill remaining positions for Offspring0 from Parent1
//    k = (r2 + 1) % CHROMOSOME_SIZE_NURSE_GA;
//    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
//        int idx = (r2 + 1 + i) % CHROMOSOME_SIZE_NURSE_GA;
//        int gene = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[1][idx];
//        if (gene >= 0 && !visited2[gene]) {
//            while (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] != -1) {
//                k = (k + 1) % CHROMOSOME_SIZE_NURSE_GA;
//            }
//            NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] = gene;
//            visited2[gene] = true;
//        }
//    }
//
//    // Fill remaining positions for Offspring1 from Parent0
//    m = (r2 + 1) % CHROMOSOME_SIZE_NURSE_GA;
//    for (i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
//        int idx = (r2 + 1 + i) % CHROMOSOME_SIZE_NURSE_GA;
//        int gene = NURSE_CROSSOVER_PARENT_STORAGE_PLACE[0][idx];
//        if (gene >= 0 && !visited1[gene]) {
//            while (NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] != -1) {
//                m = (m + 1) % CHROMOSOME_SIZE_NURSE_GA;
//            }
//            NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] = gene;
//            visited1[gene] = true;
//        }
//    }
//
//    free(visited1);
//    free(visited2);
//}

void swapMutationNurseGA(void)
{   // take the offspring from NURSE_MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2;
    for (int i = 0; i < CHROMOSOME_SIZE_NURSE_GA; i++) {
        NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[i] = NURSE_MUTATE_PARENT_STORAGE_PLACE[i];
    }

    do {
        r1 = rand() % CHROMOSOME_SIZE_NURSE_GA;
        r2 = rand() % CHROMOSOME_SIZE_NURSE_GA;
    } while (r1 == r2);

    NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r1] += NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r2] = NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r1] - NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= NURSE_MUTATED_OFFSPRING_STORAGE_PLACE[r2];
}

void generateNewChromosomeNurseGA(int chromo_num)
{
    int j, r1, r2, size = CHROMOSOME_SIZE_NURSE_GA;

    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
    for (j = 0; j < size; ++j)
        NURSE_POPULATION[chromo_num][j] = NURSE_POPULATION[chromo_num - 1][j];

    for (j = 0; j < size; ++j) {
        do {
            r1 = rand() % (size);
            r2 = rand() % (size);
        } while (r1 == r2);

        swapGenesNurseGA(chromo_num, r1, r2);
    }
}

void generatePopulationNurseGA(void)
{
    int i, j, k;
    for (j = 0; j < num_nurses; ++j)
        NURSE_POPULATION[0][j] = nurses[j].id;
    /* for (i = j; i < CHROMOSOME_SIZE_NEW_GA; ++i)
         POPULATION[0][i] = optional_patients[i - j]->id;*/
    for (i = 1; i < NURSE_POPULATION_SIZE; ++i)
        generateNewChromosomeNurseGA(i);
}

void initDataStructuresNurseGA(void)
{   //
    int i, size = CHROMOSOME_SIZE_NURSE_GA;

    // Allocate memory for single arrays
    NURSE_MUTATED_OFFSPRING_STORAGE_PLACE = (int*)calloc(size + 1, sizeof(int));
    ASSERT(NURSE_MUTATED_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for NURSE_MUTATED_OFFSPRING_STORAGE_PLACE");

    NURSE_MUTATE_PARENT_STORAGE_PLACE = (int*)calloc(size + 1, sizeof(int));
    ASSERT(NURSE_MUTATE_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for NURSE_MUTATE_PARENT_STORAGE_PLACE");

    N_G_BEST = (int*)calloc(size + 1, sizeof(int));
    ASSERT(N_G_BEST, "Dynamic Memory Allocation Error for N_G_BEST");

    // Allocate memory for crossover offspring storage
    NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[i] = (int*)calloc(size + 1, sizeof(int));
        ASSERT(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[i]");
    }

    // Allocate memory for crossover parent storage
    NURSE_CROSSOVER_PARENT_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(NURSE_CROSSOVER_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for NURSE_CROSSOVER_PARENT_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        NURSE_CROSSOVER_PARENT_STORAGE_PLACE[i] = (int*)calloc(size + 1, sizeof(int));
        ASSERT(NURSE_CROSSOVER_PARENT_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for NURSE_CROSSOVER_PARENT_STORAGE_PLACE[i]");
    }

    NURSE_POPULATION = (int**)calloc(NURSE_POPULATION_SIZE, sizeof(int*));
    ASSERT(NURSE_POPULATION, "Dynamic Memory Allocation Error for NURSE_POPULATION");

    for (i = 0; i < NURSE_POPULATION_SIZE; ++i) {
        NURSE_POPULATION[i] = (int*)calloc(size + 1, sizeof(int));
        ASSERT(NURSE_POPULATION[i], "Dynamic Memory Allocation Error for a NURSE_CHROMOSOME");

        NURSE_POPULATION[i][size] = -1;
    }
}

void freeDataStructuresNurseGA(void)
{
    int i;
    free(NURSE_MUTATE_PARENT_STORAGE_PLACE);
    free(NURSE_MUTATED_OFFSPRING_STORAGE_PLACE);

    for (i = 0; i < 2; ++i)
        free(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE[i]);
    free(NURSE_CROSSOVER_OFFSPRING_STORAGE_PLACE);

    for (i = 0; i < 2; ++i)
        free(NURSE_CROSSOVER_PARENT_STORAGE_PLACE[i]);
    free(NURSE_CROSSOVER_PARENT_STORAGE_PLACE);

    free(N_G_BEST);

    for (i = 0; i < NURSE_POPULATION_SIZE; ++i)
        free(NURSE_POPULATION[i]);
    free(NURSE_POPULATION);
}

void printPopulationNurseGA(void)
{
    int i, j;
    for (i = 0; i < NURSE_POPULATION_SIZE; ++i) {
        printf("Chromosome %d: ", i + 1);
        for (j = 0; j < CHROMOSOME_SIZE_NURSE_GA; ++j)
            printf("%d ", NURSE_POPULATION[i][j]);
        putchar('\n');
    }
}



int main(void) {
    clock_t start, end;
    double cpu_time_used;

    start = clock();  // Start time

    
    parse_json("data/instances/i05.json");
    PriorityQueue* pq;
    //srand(0);
    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
    if (!pq)
        ASSERT(0, "Dynamic Memory Allocation Error for PQ");
    size = mandatory_count;

    CHROMOSOME_SIZE_NEW_GA = size;
    CHROMOSOME_SIZE_NURSE_GA = num_nurses;
    //NURSE_POPULATION_SIZE = num_nurses;

    initDataStructuresNewGA(size);
    //initDataStructures();
    initialize_room_gender_map(&room_gender_map);
    initialize_room_shift_nurse();
    //initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
    populate_room_gender_map(&room_gender_map);
    // print_map(&room_gender_map);
     //initializePopulation();
     //generatePopulation();
     //printPopulation();
    printf("Mandatory Patients: %d\n", mandatory_count);
    printf("Optional Patients: %d\n", optional_count);
    reset_valuesNewGA();
    // update_num_patients_info();
    applyGeneticAlgorithmNewGA(pq);
    // printf("Mandatory Patients: %d\n", size);
    reset_valuesNewGA();
    append_optionalsNewGA(G_BEST);
    /*printf("\nG_BEST : \n");
    for (int i = 0; i < optional_count + mandatory_count + 2; i++) {
        printf("%d\t", G_BEST[i]);
    }*/
    //update_num_patients_info();
    ApplyHillClimbing(G_BEST, pq);
    //  ApplyHillClimbingMultiThreaded(pq);
      //update_num_patients_info();
    reset_valuesNewGA();
    admitPatientsNewGA(&room_gender_map, pq, G_BEST, mandatory_count + optional_count);
    //printf("\nBest Fitness Score: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
   // printf("\nBest optional admitted: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);

    //print_rooms();
//    freeDataStructures();
    create_dm_nurses_availability();
    sorting_nurse_id_max_load();
    create_3d_array();
    initialize_rooms_req(num_rooms);
    create_rooms_req();
    print_room_schedule();
    initDataStructuresNurseGA();
    applyNursesGA();
    resetValuesNurseGA();
    int cost = nurseAllocationGA(N_G_BEST);
    /* printf("\nBest Chromosome:");
     for (int i = 0; i <= CHROMOSOME_SIZE_NEW_GA; i++)
         printf("%d\t", G_BEST[i]);*/

    //nurse_assignments();
    create_json_file(patients, num_patients, nurses, num_nurses, num_rooms, "i05", "D:/major_code/build/output(10000)");
    //print_surgeons(surgeon);
    //
    //    // Free allocated memory
    //    // free_patients_sorted_array(sorted_mandatory_patients);
    //    // free_patients_sorted_array(sorted_optional_patients);
    free_occupants();
    //free_patients();
    free_surgeons();
    free_ots();
    free_rooms();
    free_nurses();
    //freeDataStructures();
    free(weights);

    end = clock();  // End time

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;  // Convert to seconds
    printf("Execution Time: %f seconds\n", cpu_time_used);

    return 0;
}

