#include <definition.h>

// Extern declarations (telling the compiler these are defined elsewhere)
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
extern int* room_gender_map;
//extern RoomVector* v_A, * v_B, * v_empty;

extern Nurses* nurses;
extern Shifts* shift;
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
//extern OTs* admitFromPQ_GA(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt, int* fitness_storage_place);

// --------------------ABOVE: Functions for admitting optional patients from the PQ (linked list functions as well)---------------------

//-------------------------------------------------BELOW: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

extern int k;
extern int compare_release_Day(const void* a, const void* b);

void reset_valuesNewGA(void) {
    //reset the patient structure values
    //reset rooms structure values.
    //reset the surgeon structure values.
    //reset the OT structure values.
    //reset the nurse structure values.
    //reset the occupants structure values.
    int i;

    //patient_structure
    for (i = 0; i < num_patients; i++) {
        patients[i].is_admitted = 0;
        patients[i].assigned_ot = -1;
        patients[i].assigned_room_no = -1;
        patients[i].admission_day = -1;

    }

    //room_structure
    for (i = 0; i < num_rooms; i++) {
        room[i].num_patients_allocated = 0;
        room[i].occupants_cap = 0;
        for (j=0; j<days; ++j)
            room[i].num_patients_info[j] = 0;
    }
    assign_occupants_to_rooms();

    //surgeon_structure
    for (i = 0; i < num_surgeons; i++) for (int j = 0; j < days; j++) surgeon[i].time_left[j] = surgeon[i].max_surgery_time[j];

	//OT_structure
    for (i = 0; i < num_ots; i++) for (int j = 0; j < days; j++) ot[i].time_left[j] = ot[i].max_ot_time[j];
}

void printOtDaysDataArr(OTs ***arr)
{   int d, i;
    for (d=0; d<days; ++d) {
        putchar ('\n');
        for (i=0; i<num_ots; ++i)
            printf("id: %d\tmax_ot_time: %d", arr[d][i].id, arr[d][i].max_ot_time[d]);
    }
}

int admitPatientsNewGA(int** room_gender_map, PriorityQueue* pq, int* chromosome) {
    int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, current_ot_index = 0;
    int unscheduled_mandatory = 0, scheduled_optional = 0, scheduled_mandatory = 0;
    OTs** ot_data_arr, * current_ot, **ot_days_data_arr[days];
    int p, d;

    //v_A = (RoomVector*)calloc(1, sizeof(RoomVector));
    //v_B = (RoomVector*)calloc(1, sizeof(RoomVector));
    //v_empty = (RoomVector*)calloc(1, sizeof(RoomVector));

    //make_3_vectors(room_gender_map);
    /*printVector("A", v_A);
    printVector("B", v_B);
    printVector("Empty", v_empty);*/
    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    if (ot_data_arr == NULL) {
        printf("Memory not allocated.\n");
        exit(-1);
    }
    //print_ots(ot);
    for (i = 0; i < num_ots; ++i)
        ot_data_arr[i] = ot + i;

    for (i=0; i<days; ++i) {
        ot_days_data_arr[i] = (OTs **) calloc (num_ots, sizeof(OT *));
        memcpy(ot_days_data_arr[i], ot_data_arr, num_ots * sizeof(int));
        //ot_days_data_arr[i] = ot_data_arr;
        sort_ot_data_arr(ot_days_data_arr[i], i);
    }

    // printf("\nChromosome:%d\t", k);

    int* unscheduled_mandatory_patients = (int*)calloc(num_patients, sizeof(int));
    int* scheduled_optional_patients = (int*)calloc(num_patients, sizeof(int));
    int* scheduled_mandatory_patients = (int*)calloc(num_patients, sizeof(int));

    // ------------------------------------------------------------START SCHEDULING----------------------------------------------------------------

    for (p = 0; p < CHROMOSOME_SIZE_NEW_GA; ++p) {
		s_id = patients[chromosome[p]].surgeon_id;
		p_id = chromosome[p];

        // parsing all the days in the scheduling period
        for (d = patients[p_id].surgery_release_day; d < days; ++d) {
            if (d > patients[p_id].surgery_due_day && patients[p_id].mandatory) break;
            if (surgeon[s_id].time_left[d] < patients[p_id].surgery_duration) continue;
            //updateResources(d);
            current_ot_index = 0;
            current_ot = ot_days_data_arr[d][current_ot_index];

            while (current_ot_index < num_ots && (current_ot->time_left[d] < patients[p_id].surgery_duration))
                current_ot = ot_days_data_arr[d][++current_ot_index];
            if (current_ot_index == num_ots) break;

            r_id = findSuitableRoomNewGA(p_id, d);
            if (r_id == -1) continue;
            else {
                room[r_id].num_patients_info[d]++;
                room[r_id].num_patients_allocated++;
                patients[p_id].admission_day = d;
                patients[p_id].assigned_ot = current_ot->id;
                patients[p_id].assigned_room_no = r_id;
                patients[p_id].is_admitted = 1;
                surgeon[s_id].time_left[d] -= patients[p_id].surgery_duration;
                ot[current_ot->id].time_left[d] -= patients[p_id].surgery_duration;
                ot_days_data_arr[d][current_ot_index].time_left[d] -= patients[p_id].surgery_duration;
                break;
            }
        }
    }

    for (i = 0; i < num_patients; i++) {
        if (patients[i].admission_day == -1 && patients[i].mandatory) {
            unscheduled_mandatory++;
            unscheduled_mandatory_patients[patients[i].id] = 1;
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
            continue;
        }
    }

    free(unscheduled_mandatory_patients);
    free(scheduled_mandatory_patients);
    free(scheduled_optional_patients);
    //free(v_A);
    //free(v_B);
    //free(v_empty);

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
    // Returns a room_id OR -1 if no suitable room is found
    int flag = 0, j, r_id = -1, room_to_be_assigned = -1;
    gender g = patients[p_id].gen, r_gen;
    char* age = patients[p_id].age_group;

    for (int i = 0; i < num_rooms; i++) {
        flag = 0;
        r_id = room[i].id;
        r_gen = room[i].gender_days_info[d];
        if (r_gen != g) continue;

        // Check if room is full
        if ((room[r_id].cap <= room[r_id].num_patients_info[d]) continue;
        // Check for incompatible rooms
        for (j = 0; j < patients[p_id].num_incompatible_rooms; j++) {
            if (r_id == patients[p_id].incompatible_room_ids[j]) {
                flag = 1; // Mark as incompatible
                break;
            }
        }
        if (flag) continue;
		for (j = d; j < d + patients[p_id].length_of_stay && j < days; ++j) {
			if (room[r_id].cap <= room[r_id].num_patients_info[j]) {
				flag = 1;
				break;
			}
		}
		if (flag) continue;
		room_to_be_assigned = r_id;
		return room_to_be_assigned; // Found a suitable room
    }

    // check in empty rooms
    if (room_to_be_assigned == -1 || i == num_rooms) {
        for (int i = 0; i < num_rooms; i++) {
        flag = 0;
        r_id = room[i].id;
        r_gen = room[i].gender_days_info[d];
        if (r_gen != -1)
            continue;

        // Check if room is full
        if ((room[r_id].cap <= room[r_id].num_patients_info[d]) continue;
        // Check for incompatible rooms
        for (j = 0; j < patients[p_id].num_incompatible_rooms; j++) {
            if (r_id == patients[p_id].incompatible_room_ids[j]) {
                flag = 1; // Mark as incompatible
                break;
            }
        }
        if (flag) continue;
		for (j = d; j < d + patients[p_id].length_of_stay && j < days; ++j) {
			if (room[r_id].cap <= room[r_id].num_patients_info[j]) {
				flag = 1;
				printf("Dikkat hai re baba!!");
				exit(EXIt_FAILURE);
				break;
			}
		}
        if (flag) continue;
            room_to_be_assigned = r_id;
            room[room_to_be_assigned].gender_days_info[d] = g;
            return room_to_be_assigned; // Found a suitable room
        }
    }
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
    return -1; // No suitable room found
}

//----------------------------------------------------ABOVE: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

//-----------------------------------------------------------BELOW: GENETIC ALGORITHM-----------------------------------------------------------------

//-------------------------------------------------------BELOW: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

extern void swapGenes(int, int, int);

void applyGeneticAlgorithmNewGA(PriorityQueue* pq);
void initDataStructuresNewGA(void);
void freeDataStructuresNewGA(void);
void evaluateFitnessScoreNewGA(int* chromosome, PriorityQueue* pq);
void crossoverTournamentSelectionNewGA(void);
void mutationTournamentSelectionNewGA(void);
void generatePopulationNewGA(void);
void mutationElitismViolations(void);
void crossoverElitismViolations(void);
void mutationElitismCost(void);
void crossoverElitismCost(void);
void minimizeCost(void); // returns the minimized cost
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

const long int NUM_COST_ITER = 10000000;
const int POPULATION_SIZE = 1000, NUM_ITER = 10000, CONVERGENCE_STOPPING_CRITERION = 100;
int CHROMOSOME_SIZE_NEW_GA;
int** POPULATION, * G_BEST, size;
int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE;

//-------------------------------------------------------ABOVE: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void printPopulationNewGA(void)
{   int i, j;
    for (i = 0; i < POPULATION_SIZE; ++i) {
        printf("Chromosome %d: ", i + 1);
        for (j = 0; j < CHROMOSOME_SIZE_NEW_GA; ++j)
            printf("%d ", POPULATION[i][j]);
        putchar('\n');
    }
}

void orderCrossoverNewGA(void) {
    int r1, r2, i, k, m;
    int max = 0;

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

    for (i = 0; i < CHROMOSOME_SIZE_NEW_GA; i++) {
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
            while (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] != -1) {
                k = (k + 1) % CHROMOSOME_SIZE_NEW_GA;
            }
            CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] = gene1;
            visited1[gene1] = true;
        }

        int gene2 = CROSSOVER_PARENT_STORAGE_PLACE[0][index];
        if (!visited2[gene2]) {
            while (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] != -1) {
                m = (m + 1) % CHROMOSOME_SIZE_NEW_GA;
            }
            CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] = gene2;
            visited2[gene2] = true;
        }
    }

	CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA] = -1;
	CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA+1] = -1;
    CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA] = -1;
    CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA+1] = -1;

    free(visited1);
    free(visited2);
}

void swapMutationNewGA(void)
{   // take the offspring from MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2;
    for (int i = 0; i < CHROMOSOME_SIZE_NEW_GA; i++) {
        MUTATED_OFFSPRING_STORAGE_PLACE[i] = MUTATE_PARENT_STORAGE_PLACE[i];
    }

    do {
        r1 = rand() % CHROMOSOME_SIZE_NEW_GA;
        r2 = rand() % CHROMOSOME_SIZE_NEW_GA;
    } while (r1 == r2);

    MUTATED_OFFSPRING_STORAGE_PLACE[r1] += MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r2] = MUTATED_OFFSPRING_STORAGE_PLACE[r1] - MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= MUTATED_OFFSPRING_STORAGE_PLACE[r2];

	MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] = -1;
	MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA + 1] = -1;
}

void generateNewChromosomeNewGA(int chromo_num)
{
    int j, r1, r2;

    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
    for (j = 0; j < CHROMOSOME_SIZE_NEW_GA; ++j)
        POPULATION[chromo_num][j] = POPULATION[chromo_num - 1][j];

    for (j = 0; j < CHROMOSOME_SIZE_NEW_GA; ++j) {
        do {
            r1 = rand() % (CHROMOSOME_SIZE_NEW_GA);
            r2 = rand() % (CHROMOSOME_SIZE_NEW_GA);
        } while (r1 == r2);

        swapGenes(chromo_num, r1, r2);
    }
}

void initDataStructuresNewGA(void)
{
    int i;

    // Allocate memory for single arrays
    MUTATED_OFFSPRING_STORAGE_PLACE = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
    ASSERT(MUTATED_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for MUTATED_OFFSPRING_STORAGE_PLACE");

    MUTATE_PARENT_STORAGE_PLACE = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
    ASSERT(MUTATE_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for MUTATE_PARENT_STORAGE_PLACE");

    G_BEST = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
    ASSERT(G_BEST, "Dynamic Memory Allocation Error for G_BEST");

    // Allocate memory for crossover offspring storage
    CROSSOVER_OFFSPRING_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(CROSSOVER_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        CROSSOVER_OFFSPRING_STORAGE_PLACE[i] = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
        ASSERT(CROSSOVER_OFFSPRING_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE[i]");
    }

    // Allocate memory for crossover parent storage
    CROSSOVER_PARENT_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(CROSSOVER_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        CROSSOVER_PARENT_STORAGE_PLACE[i] = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
        ASSERT(CROSSOVER_PARENT_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE[i]");
    }

    POPULATION = (int**)calloc(POPULATION_SIZE, sizeof(int*));
    ASSERT(POPULATION, "Dynamic Memory Allocation Error for POPULATION");

    for (i = 0; i < POPULATION_SIZE; ++i) {
        POPULATION[i] = (int*)calloc(CHROMOSOME_SIZE_NEW_GA + 2, sizeof(int));
        ASSERT(POPULATION[i], "Dynamic Memory Allocation Error for a CHROMOSOME");

        // Set last column to -1 if needed
        POPULATION[i][CHROMOSOME_SIZE_NEW_GA] = -1;
        POPULATION[i][CHROMOSOME_SIZE_NEW_GA+1] = -1;
    }
}

void freeDataStructuresNewGA(void)
{   int i;
    free(MUTATED_OFFSPRING_STORAGE_PLACE);
    free(MUTATED_OFFSPRING_STORAGE_PLACE);

    for (i = 0; i < 2; ++i)
        free(CROSSOVER_OFFSPRING_STORAGE_PLACE[i]);
    free(CROSSOVER_OFFSPRING_STORAGE_PLACE);

    for (i = 0; i < 2; ++i)
        free(CROSSOVER_PARENT_STORAGE_PLACE[i]);
    free(CROSSOVER_PARENT_STORAGE_PLACE);

    free(G_BEST);

    for (i = 0; i < POPULATION_SIZE; ++i)
        free(POPULATION[i]);
    free(POPULATION);
}

void applyGeneticAlgorithmNewGA(PriorityQueue* pq)
{   /*
    In this implementation of GA:
    1. We have a population of size POPULATION_SIZE.
    2. Each chromosome in the population is of size CHROMOSOME_SIZE_NEW_GA.
    3. We have a crossover probability p_c = 0.85.
    4. We have a mutation probability p_m = 0.15.
    5. We have a tournament size of 2.
    6. We have a crossover point of 2.
    7. We have a mutation point of 1.
    8. We have a best chromosome G_BEST.
    9. We have a crossover offspring storage place of size 2.
    10. We have a crossover parent storage place of size 2.
    11. We have a mutated offspring storage place of size 1.
    12. We have a mutated parent storage place of size 1.
    13. We have a number of iterations NUM_ITER = 10000.
    14. We have a convergence stopping criterion CONVERGENCE_STOPPING_CRITERION = 100.
    15. We have a same_fitness_iter variable to keep track of the number of iterations the best fitness score remains the same.
    16. We have a best_fitness variable to keep track of the best fitness score.
    17. We have a g_best variable to keep track of the best fitness score.

    18. The last 2 columns of the chromosome are used to store the fitness score of the chromosome.
        18.1 . The second last column stores the #mandatory_patients admitted.
        18.2 . The last column stores the fitness score of the chromosome, i.e. TOTAL #PATIENTS ADMITTED (MANDAOTRY + OPTIONAL).
    */
    int i, j, best_fitness, g_best, best_fitness_mandatory = 0, minimized_cost;
    //int fitness_storage_place[2]; // This array is used to store the 2 fitness scores of the chromosome.
    unsigned int same_fitness_iter;
    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
    generatePopulationNewGA();

    for (i = 0; i < POPULATION_SIZE; ++i)
        evaluateFitnessScoreNewGA(POPULATION[i], pq);
    //printf("%d\n", POPULATION[0]);

    //printPopulation();
    // GBEST: The best chromosome in the population. Currently - the first chromosome.
    memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
    G_BEST[CHROMOSOME_SIZE_NEW_GA] = -1;
    G_BEST[CHROMOSOME_SIZE_NEW_GA+1] = -1;
    /*for (int i = 0; i < CHROMOSOME_SIZE_NEW_GA; i++) {
        G_BEST[i] = POPULATION[0][i];
    }*/
    g_best = G_BEST[CHROMOSOME_SIZE_NEW_GA];
    //printPopulation();
    //ot overtime
    //room capacity
    //gender mix

    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    // increase the NUM_ITER if g_best doesn't converge to 0
    // still can't converge? - change the crossover / mutation methods.
    for (i = 0; i <= NUM_ITER && g_best; ++i) {
        if ((rand() / (float)RAND_MAX) <= p_c) {
            crossoverTournamentSelectionNewGA();
            orderCrossoverNewGA();

            for (j = 0; j < 2; ++j)
                evaluateFitnessScoreNewGA(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq);
            crossoverElitismViolations();
            for (j = 0; j < 2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA] <= G_BEST[CHROMOSOME_SIZE_NEW_GA])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
        }
        else {
            mutationTournamentSelectionNewGA();
            swapMutationNewGA();
            evaluateFitnessScoreNewGA(MUTATED_OFFSPRING_STORAGE_PLACE, pq);
            mutationElitismViolations();
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] <= G_BEST[CHROMOSOME_SIZE_NEW_GA])
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        //best_fitness = G_BEST[CHROMOSOME_SIZE_NEW_GA];
        //best_fitness_mandatory = G_BEST[CHROMOSOME_SIZE_NEW_GA];
        /*if (best_fitness == g_best)
            same_fitness_iter++;
        else
            same_fitness_iter = 0;*/
        g_best = G_BEST[CHROMOSOME_SIZE_NEW_GA];
    }

    printf("\nViolations in G_BEST: %d", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
    if (!G_BEST[CHROMOSOME_SIZE_NEW_GA]) {
        puts("\nCouldn't find a chromosome with 0 violations.\nExiting...");
        exit(EXIT_FAILURE);
    }

    minimizeCost(pq);
    printf("\Cost of G_BEST: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA+1]);

    //puts("\nChecking whether the iterations were over OR the algorithm converged: ");
    printf("Number of iterations: %d", i);
    // either i will be NUM_ITER (1000000) OR same_fitness_iter will be CONVERGENCE_STOPPING_CRITERION (100).
    // G_BEST is the best chromosome - use it for admitting patients.
}

void evaluateFitnessScoreNewGA(int* chromosome, PriorityQueue* pq)
{   // fitness score is the number of mandatory patients who were admitted in the scheduling period.
    // The best fitness score is CHROMOSOME_SIZE_NEW_GA
    reset_valuesNewGA();
    chromosome[CHROMOSOME_SIZE_NEW_GA] = admitPatientsNewGA(&room_gender_map, pq, chromosome);

    //chromosome[CHROMOSOME_SIZE_NEW_GA] = findViolations();
    chromosome[CHROMOSOME_SIZE_NEW_GA + 1] = findCost();
    //reset_valuesNewGA();

    //empty_pq(pq);
}

void minimizeCost(PriorityQueue* pq)
{   int i, j, r1, r2;
    int *TEMP_G_BEST;
    TEMP_G_BEST = (int *) calloc (CHROMOSOME_SIZE_NEW_GA+2, sizeof(int));
    memcpy(TEMP_G_BEST, G_BEST, (CHROMOSOME_SIZE_NEW_GA+2) * sizeof(int));
    TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA] = -1;
    TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA+1] = -1;

    for (i=0; i<NUM_COST_ITER && G_BEST[CHROMOSOME_SIZE_NEW_GA+1]; ++i) {
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

        evaluateFitnessScoreNewGA(TEMP_G_BEST, pq);
        if (TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA]) {
            memcpy(TEMP_G_BEST, G_BEST, (CHROMOSOME_SIZE_NEW_GA+2) * sizeof(int));
            continue;
        }
        //replace G_BEST if the cost of TEMP_G_BEST is less than that of the G_BEST
        if (TEMP_G_BEST[CHROMOSOME_SIZE_NEW_GA+1] < G_BEST[CHROMOSOME_SIZE_NEW_GA+1])
            memcpy(G_BEST, TEMP_G_BEST, (CHROMOSOME_SIZE_NEW_GA+2) * sizeof(int));
    }
    /*
    We can parallelize this process by first finding 4 chromosomes that have 0 violations and then putting each of them on one thread/core and
    running this loop for each of them.
    */
}

int findViolations(void) {
    // add validator code here
    return 0;
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
int patient_delay(){
	int cost = 0;
	for(int p = 0 ; p < num_patients ; p++){
		if(patients[p].admission_day != -1 && patients[p].admission_day > patients[p].surgery_release_day){
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
    char *p;

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
                    if (occupant[id].skill_level_required[s] > nurses[n].skill_level) {
                        cost += occupant[id].skill_level_required[s] - nurses[n].skill_level;
                    }
                }
            }
        }
    }
    return cost;
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
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA+1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA+1])
            worst_fitness_chromosome_index = i;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE_NEW_GA+1] > POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA+1] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE_NEW_GA+1] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA+1])
        memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
    else {
        if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA+1] <= POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA+1])
            memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
        return;
    }
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE_NEW_GA+1] <= POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE_NEW_GA+1])
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

void generatePopulationNewGA(void)
{   int i, j, k;
    /*
    for (j = 0; j < mandatory_count; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;
    for (i = j; i < CHROMOSOME_SIZE_NEW_GA; ++i)
        POPULATION[0][i] = optional_patients[i - j]->id;
    */
    for (i = 0; i < num_patients; ++i)
        POPULATION[0][i] = patients[i]->id;

    for (i = 1; i < POPULATION_SIZE; ++i)
        generateNewChromosomeNewGA(i);
}

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

int main(void) {
    parse_json("data/instances/i08.json");
    PriorityQueue* pq;
    //srand(0);
    //pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
    if (!pq)
        ASSERT(0, "Dynamic Memory Allocation Error for PQ");
    size = mandatory_count + optional_count;

    CHROMOSOME_SIZE_NEW_GA = size;

    initDataStructuresNewGA();
    initialize_room_gender_map(&room_gender_map);
    //initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
    populate_room_gender_map(&room_gender_map);
    print_map(&room_gender_map);
    //initializePopulation();
    //generatePopulation();
    //printPopulation();
    printf("Mandatory Patients: %d\n", mandatory_count);
    printf("Optional Patients: %d\n", optional_count);
    reset_valuesNewGA();
    applyGeneticAlgorithmNewGA(pq);
    // printf("Mandatory Patients: %d\n", size);
    reset_valuesNewGA();
    admitPatientsNewGA(&room_gender_map, pq, G_BEST);
    printf("\nBest Fitness Score: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
    printf("\nBest optional admitted: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);

    create_dm_nurses_availability();
    sorting_nurse_id_max_load();
    create_3d_array();
    initialize_rooms_req(num_rooms);
    create_rooms_req();
    printf("\nBest Chromosome:");
    for (int i = 0; i <= CHROMOSOME_SIZE_NEW_GA; i++)
        printf("%d\t", G_BEST[i]);

    nurse_assignments();
    create_json_file(patients, num_patients, nurses, num_nurses, num_rooms, "i08", "D:/major_code/build/output");

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
    freeDataStructuresNewGA();
    free(weights);

    return 0;
}
