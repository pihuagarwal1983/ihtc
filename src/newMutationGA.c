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
extern char**** room_schedule;
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
extern int** room_shift_nurse;
extern void initialize_room_shift_nurse();
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
extern void reset_valuesNewGA(void);
extern void printOtDaysDataArr(OTs*** arr);
extern int admitPatientsNewGA(int** room_gender_map, PriorityQueue* pq, int* chromosome,int size); 
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

extern int findSuitableRoomNewGA(int p_id, int d); 
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

extern void swapGenes(int, int, int);

//void applyGeneticAlgorithmNewGA(PriorityQueue* pq);
//void initDataStructuresNewGA(void);
//void freeDataStructuresNewGA(void);
//void evaluateFitnessScoreNewGA(int* chromosome, PriorityQueue* pq);
//void crossoverTournamentSelectionNewGA(void);
//void mutationTournamentSelectionNewGA(void);
//void generatePopulationNewGA(void);
//void mutationElitismViolations(void);
//void crossoverElitismViolations(void);
//void mutationElitismCost(void);
//void crossoverElitismCost(void);
//void minimizeCost(void); // returns the minimized cost
//void orderCrossoverNewGA(void);
//void printPopulationNewGA(void);
//void swapMutationNewGA(int);
//void generateNewChromosomeNewGA(int chromo_num);

extern const long int NUM_COST_ITER;
extern const int POPULATION_SIZE, NUM_ITER, CONVERGENCE_STOPPING_CRITERION;
extern int CHROMOSOME_SIZE_NEW_GA;
extern int** POPULATION, * G_BEST, size;
extern int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
extern int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE;
extern void evaluateFitnessScoreNewGA(int* chromosome, PriorityQueue* pq,int size);
extern int findViolations(void);

//..........COST FUNCTIONS..........//
extern int patient_delay();
extern int elective_unscheduled_patients();
extern int open_operating_theatre();
extern int room_age_mix();
extern int skill_level_mix();
extern void crossoverTournamentSelectionNewGA(void);
extern void mutationTournamentSelectionNewGA(void);
extern int findCost(void);

extern void crossoverElitismViolations(void);
extern void mutationElitismViolations(void);
extern void crossoverElitismCost(void);
extern void mutationElitismCost(void);
extern void generatePopulationNewGA(void);

//-------------------------------------------------------ABOVE: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

extern void printPopulationNewGA(void);
extern void orderCrossoverNewGA(void);

void swapMutationNewMutation(int iter)
{   // take the offspring from MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2, i, j;
    float p_m = 0.9;
	 p_m = abs((iter / NUM_ITER) - 0.9);
    /*
    iter == 0 -> p_m = 90% biased mutation
    iter == NUM_ITER -> p_m = 10% biased mutation
    */
    
    memcpy(MUTATED_OFFSPRING_STORAGE_PLACE, MUTATE_PARENT_STORAGE_PLACE, CHROMOSOME_SIZE_NEW_GA * sizeof(int));

    do {
        r1 = rand() % CHROMOSOME_SIZE_NEW_GA;
        r2 = rand() % CHROMOSOME_SIZE_NEW_GA;
    } while (r1 >= r2);

    if ((rand() / (float)RAND_MAX) <= p_m) {
        // perform the biased mutation
        if (patients[MUTATED_OFFSPRING_STORAGE_PLACE[r1]].mandatory) {
            // select the nearest optional patient
            for (i = r1 - 1; i >= 0; --i)
                if (!patients[MUTATED_OFFSPRING_STORAGE_PLACE[i]].mandatory) {
                    r1 = i;
                    break;
                }
            if (i == -1)
                for (i = r1 + 1; i < r2; ++i)
                    if (!patients[MUTATED_OFFSPRING_STORAGE_PLACE[i]].mandatory) {
                        r1 = i;
                        break;
                    }
            if (i == r2)
                ASSERT(0, "gadbad hai re baba!!");
        }

        if (!patients[MUTATED_OFFSPRING_STORAGE_PLACE[r2]].mandatory) {
            // select the nearest mandatory patient
            for (i = r2 + 1; i < CHROMOSOME_SIZE_NEW_GA; ++i)
                if (patients[MUTATED_OFFSPRING_STORAGE_PLACE[i]].mandatory) {
                    r2 = i;
                    break;
                }
            if (i == CHROMOSOME_SIZE_NEW_GA)
                for (i = r2 - 1; i > r1; --i)
                    if (patients[MUTATED_OFFSPRING_STORAGE_PLACE[i]].mandatory) {
                        r2 = i;
                        break;
                    }
            if (i == r1)
                ASSERT(0, "gadbad hai re baba!!");
        }
		// perform the ordinary mutation
		MUTATED_OFFSPRING_STORAGE_PLACE[r1] += MUTATED_OFFSPRING_STORAGE_PLACE[r2];
		MUTATED_OFFSPRING_STORAGE_PLACE[r2] = MUTATED_OFFSPRING_STORAGE_PLACE[r1] - MUTATED_OFFSPRING_STORAGE_PLACE[r2];
		MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    }

    else {
        // perform the ordinary mutation
        MUTATED_OFFSPRING_STORAGE_PLACE[r1] += MUTATED_OFFSPRING_STORAGE_PLACE[r2];
        MUTATED_OFFSPRING_STORAGE_PLACE[r2] = MUTATED_OFFSPRING_STORAGE_PLACE[r1] - MUTATED_OFFSPRING_STORAGE_PLACE[r2];
        MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    }

	MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA] = -1;
	MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE_NEW_GA + 1] = -1;
}

extern void generateNewChromosomeNewGA(int chromo_num);
extern void initDataStructuresNewGA(int);
extern void minimizeCost(PriorityQueue* pq);
extern void freeDataStructuresNewGA(void);

void applyGeneticAlgorithmNewMutation(PriorityQueue* pq)
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

    for (i = 0; i < POPULATION_SIZE; ++i) {
        evaluateFitnessScoreNewGA(POPULATION[i], pq, CHROMOSOME_SIZE_NEW_GA);
        //printf("%d\n", POPULATION[0]);

        //printPopulation();
        // GBEST: The best chromosome in the population. Currently - the first chromosome.
        memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE_NEW_GA + 2) * sizeof(int));
        G_BEST[CHROMOSOME_SIZE_NEW_GA] = POPULATION[0][CHROMOSOME_SIZE_NEW_GA];
        G_BEST[CHROMOSOME_SIZE_NEW_GA + 1] = POPULATION[0][CHROMOSOME_SIZE_NEW_GA + 1];
    }
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
                evaluateFitnessScoreNewGA(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq, CHROMOSOME_SIZE_NEW_GA);
            crossoverElitismViolations();
            for (j = 0; j < 2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE_NEW_GA] <= G_BEST[CHROMOSOME_SIZE_NEW_GA])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE_NEW_GA + 2));
        }
        else {
            mutationTournamentSelectionNewGA();
            swapMutationNewMutation(i);
            evaluateFitnessScoreNewGA(MUTATED_OFFSPRING_STORAGE_PLACE, pq, CHROMOSOME_SIZE_NEW_GA);
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
        if (i == 100000) {
            printf("Pausing......");
            getchar();
        }
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



//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

//int main(void) {
//    parse_json("data/instances/i08.json");
//    PriorityQueue* pq;
//    //srand(0);
//    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
//    if (!pq)
//        ASSERT(0, "Dynamic Memory Allocation Error for PQ");
//
//    CHROMOSOME_SIZE_NEW_GA = num_patients;
//
//    initDataStructuresNewGA(CHROMOSOME_SIZE_NEW_GA);
//    initialize_room_gender_map(&room_gender_map);
//    //initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
//    populate_room_gender_map(&room_gender_map);
//    print_map(&room_gender_map);
//    initialize_room_shift_nurse();
//    //initializePopulation();
//    //generatePopulation();
//    //printPopulation();
//    printf("Mandatory Patients: %d\n", mandatory_count);
//    printf("Optional Patients: %d\n", optional_count);
//    reset_valuesNewGA();
//    applyGeneticAlgorithmNewMutation(pq);
//    // printf("Mandatory Patients: %d\n", size);
//    reset_valuesNewGA();
//    admitPatientsNewGA(&room_gender_map, pq, G_BEST, CHROMOSOME_SIZE_NEW_GA);
//    printf("\nBest Fitness Score: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA]);
//    printf("\nBest optional admitted: %d\n", G_BEST[CHROMOSOME_SIZE_NEW_GA + 1]);
//
//    create_dm_nurses_availability();
//    sorting_nurse_id_max_load();
//    create_3d_array();
//    initialize_rooms_req(num_rooms);
//    create_rooms_req();
//    printf("\nBest Chromosome:");
//    for (int i = 0; i <= CHROMOSOME_SIZE_NEW_GA; i++)
//        printf("%d\t", G_BEST[i]);
//
//    nurse_assignments();
//    create_json_file(patients, num_patients, nurses, num_nurses, num_rooms, "i08", "D:/major_code/build/output");
//
//    //
//    //    // Free allocated memory
//    //    // free_patients_sorted_array(sorted_mandatory_patients);
//    //    // free_patients_sorted_array(sorted_optional_patients);
//    free_occupants();
//    //free_patients();
//    free_surgeons();
//    free_ots();
//    free_rooms();
//    free_nurses();
//    //freeDataStructuresNewGA();
//    free(weights);
//
//    return 0;
//}
