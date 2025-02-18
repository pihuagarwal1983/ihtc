//#include <stdio.h>
////#include <ncurses.h>
////#include <conio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <math.h>
//#include <time.h>
//#include "cJSON.h"
//#include <direct.h>
//#include <stdbool.h>
//#include <cASSERT>

#include <definition.h>
//#include "side_c-checkpoint.c"

//#define ASSERT(condition, message) \
//    do { \
//        if (!(condition)) { \
//            fprintf(stderr, "ASSERTion failed: %s\nFile: %s, Line: %d\n", message, __FILE__, __LINE__); \
//            exit(EXIT_FAILURE); \
//        } \
//    } while (0)

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
//int num_gender_A_rooms, num_gender_B_rooms, num_empty_rooms;
//int* current_size_dm_nurse;
extern int* room_gender_map;

typedef struct {
    HeapNode** data;
    int size;
    int capacity;
} Mand_opt_PQ;
extern RoomVector* v_A, * v_B, * v_empty;

extern const int POPULATION_SIZE , NUM_ITER , CONVERGENCE_STOPPING_CRITERION ;
extern int** POPULATION, * G_BEST, CHROMOSOME_SIZE; int size;
extern int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
extern int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, * chromosome;

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

//void reset_Values();
//void admit_optional_patients(int**, OTs**);
void generatePopulation2(void);


extern void empty_pq(PriorityQueue* pq); 

extern void update_LOS_of_patients_GA(int d, int* chromosome);
extern OTs* admitFromPQ_GA(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt, int* fitness_storage_place);
// --------------------ABOVE: Functions for admitting optional patients from the PQ (linked list functions as well)---------------------

//-------------------------------------------------BELOW: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

extern int k ;

extern void reset_values(); 

extern int compare_release_Day(const void* a, const void* b); 

void admitPatientsGA2(int** room_gender_map, PriorityQueue* pq, int* chromosome) {
    int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, day, p_counter;
    int unscheduled_mandatory = 0, scheduled_optional_count = 0, assigned_ot, flag, max = 0;
    OTs** ot_data_arr, * current_ot;

    v_A = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_B = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_empty = (RoomVector*)calloc(1, sizeof(RoomVector));

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
    for (int i = 0; i < CHROMOSOME_SIZE; i++)
        if (max < chromosome[i])
            max = chromosome[i];

    int* unscheduled_mandatory_patients = (int*)calloc(max + 1, sizeof(int));
    int* scheduled_optional_patients = (int*)calloc(max + 1, sizeof(int));
    for (day = 0; day < days; day++) {
        if (day)
            update_LOS_of_patients_GA(day, chromosome);
        current_ot_index = 0;
        sort_ot_data_arr(ot_data_arr, day);
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
                            //insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
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
                                else continue;
                                //ot_data_arr[current_ot_index]->time_left[day] -= patients[p_id].surgery_duration;
                            }
                            else {
                                // if true - it means no ot could be assigned to this patient - put him in PQ
                                //insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
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
                            if (!patients[p_id].mandatory) {
								scheduled_optional_patients[p_id] = 1;
								scheduled_optional_count++;
                            }

                        }
                        else continue;
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
    free(v_A);
    free(v_B);
    free(v_empty);

}

//void admitPatientsGA2(int** room_gender_map, PriorityQueue* pq, int* chromosome) {
//    int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, day, p_counter;
//    int unscheduled_mandatory = 0, scheduled_optional_count = 0, assigned_ot, flag, max = 0;
//    OTs** ot_data_arr, * current_ot;
//
//    v_A = (RoomVector*)calloc(1, sizeof(RoomVector));
//    v_B = (RoomVector*)calloc(1, sizeof(RoomVector));
//    v_empty = (RoomVector*)calloc(1, sizeof(RoomVector));
//
//    make_3_vectors(room_gender_map);
//    /*printVector("A", v_A);
//    printVector("B", v_B);
//    printVector("Empty", v_empty);*/
//    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
//    if (ot_data_arr == NULL) {
//        printf("Memory not allocated.\n");
//        exit(-1);
//    }
//    //print_ots(ot);
//    for (i = 0; i < num_ots; ++i)
//        ot_data_arr[i] = ot + i;
//
//    // printf("\nChromosome:%d\t", k);
//    for (int i = 0; i < CHROMOSOME_SIZE; i++)
//        if (max < chromosome[i])
//            max = chromosome[i];
//
//    int* unscheduled_mandatory_patients = (int*)calloc(max + 1, sizeof(int));
//    int* scheduled_optional_patients = (int*)calloc(max + 1, sizeof(int));
//
//    for (day = 0; day < days; day++) {
//        if (day)
//            update_LOS_of_patients_GA(day, chromosome);
//        current_ot_index = 0;
//        sort_ot_data_arr(ot_data_arr, day);
//        current_ot = ot_data_arr[current_ot_index];
//        for (p_counter = 0; p_counter < CHROMOSOME_SIZE; p_counter++) {
//            p_id = chromosome[p_counter];
//
//            if (patients[p_id].surgery_release_day > day || patients[p_id].admission_day != -1) {
//                continue;
//            }
//            else {
//                
//                // for optional patients
//                if (patients[p_id].surgery_due_day == -1) {
//					if (patients[p_id].surgery_release_day <= day) {
//						// check if the OT is available
//						if (current_ot->time_left[day] < patients[p_id].surgery_duration) {
//							for (j = 0; j < num_ots || assigned_ot == -1; j++) {
//								if (ot_data_arr[j]->time_left[day] >= patients[p_id].surgery_duration) {
//									assigned_ot = ot_data_arr[j]->id;
//									break;
//								}
//							}
//							if (assigned_ot != -1) {
//								if (j < num_ots) {
//									current_ot_index = j;
//									current_ot = ot_data_arr[current_ot_index];
//									assigned_ot = current_ot->id;
//								}
//								else continue;
//								//ot_data_arr[current_ot_index]->time_left[day] -= patients[p_id].surgery_duration;
//							}
//							else {
//								// if true - it means no ot could be assigned to this patient - put him in PQ
//								//insertNodeInPQ(pq, makeHeapNode(p_id, 0, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//								continue;
//							}
//						}
//						else {
//							assigned_ot = current_ot->id;
//						}
//						// if the control comes here - that means surgeon and OT are available and only room remains
//						// look for a suitable room
//						r_id = (patients[p_id].gen) ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
//						if (r_id != -1 && surgeon[patients[p_id].surgeon_id].time_left[day] >= patients[p_id].surgery_duration) {
//							room[r_id].num_patients_allocated++;
//							patients[p_id].admission_day = day;
//							patients[p_id].assigned_ot = assigned_ot;
//							patients[p_id].assigned_room_no = r_id;
//							patients[p_id].is_admitted = 1;
//							surgeon[patients[p_id].surgeon_id].time_left[day] -= patients[p_id].surgery_duration;
//							ot[assigned_ot].time_left[day] -= patients[p_id].surgery_duration;
//
//							scheduled_optional_patients[p_id] = 1;
//                            scheduled_optional_count++;
//						}
//                        else {
//                            r_id = -1;
//							assigned_ot = -1;
//                            patients[p_id].admission_day = -1;
//                            continue;
//                        }
//					}
//					continue;
//				}
//				// for mandatory patients
//
//                if (patients[p_id].surgery_due_day < day && patients[p_id].admission_day == -1 && unscheduled_mandatory_patients[p_id] == 0) {
//                    ++unscheduled_mandatory;
//                    unscheduled_mandatory_patients[p_id] = 1;
//                    continue;
//                }
//                else {
//                    if (patients[p_id].surgery_release_day <= day && patients[p_id].surgery_due_day >= day) {
//                        s_id = patients[p_id].surgeon_id;
//                        // check if the surgeon is available
//                        if (surgeon[s_id].time_left[day] < patients[p_id].surgery_duration) {
//                            //insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//                            continue;
//                        }
//                        // check if the OT is available
//                        if (current_ot->time_left[day] < patients[p_id].surgery_duration) {
//                            for (j = 0; j < num_ots || assigned_ot == -1; j++) {
//                                if (ot_data_arr[j]->time_left[day] >= patients[p_id].surgery_duration) {
//                                    assigned_ot = ot_data_arr[j]->id;
//                                    break;
//                                }
//                            }
//                            if (assigned_ot != -1) {
//                                if (j < num_ots) {
//                                    current_ot_index = j;
//                                    current_ot = ot_data_arr[current_ot_index];
//                                    assigned_ot = current_ot->id;
//                                }
//                                else continue;
//                                //ot_data_arr[current_ot_index]->time_left[day] -= patients[p_id].surgery_duration;
//                            }
//                            else {
//                                // if true - it means no ot could be assigned to this patient - put him in PQ
//                                //insertNodeInPQ(pq, makeHeapNode(p_id, 1, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//                                continue;
//                            }
//                        }
//                        else {
//                            assigned_ot = current_ot->id;
//                        }
//
//                        // if the control comes here - that means surgeon and OT are available and only room remains
//                        // look for a suitable roo
//                        r_id = (patients[p_id].gen) ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
//                        if (r_id != -1 && surgeon[patients[p_id].surgeon_id].time_left[day] >= patients[p_id].surgery_duration) {
//                            room[r_id].num_patients_allocated++;
//                            patients[p_id].admission_day = day;
//                            patients[p_id].assigned_ot = assigned_ot;
//                            patients[p_id].assigned_room_no = r_id;
//                            patients[p_id].is_admitted = 1;
//                            surgeon[patients[p_id].surgeon_id].time_left[day] -= patients[p_id].surgery_duration;
//                            ot[assigned_ot].time_left[day] -= patients[p_id].surgery_duration;
//
//                        }
//                        else {
//                            r_id = -1;
//                            assigned_ot = -1;
//                            patients[p_id].admission_day = -1;
//                            continue;
//                        }
//                    }
//                }
//            }
//        }
//    }
//    //print_surgeons(surgeon);
//    //print_ots(ot);
//    //print_rooms();
//   // printVector("Empty", v_empty);
//
//
//    for (int i = 0; i < max+1; i++) {
//        if (patients[i].admission_day == -1 && patients[i].mandatory) {
//            unscheduled_mandatory++;
//            unscheduled_mandatory_patients[i] = 1;
//        }
//    }
//
//    for (int i = 0; i < max + 1; i++) {
//        if (patients[i].admission_day != -1 && !patients[i].mandatory) {
//            scheduled_optional_count++;
//            scheduled_optional_patients[i] = 1;
//        }
//    }
//
//	// put the fitness values in the right place
//	chromosome[CHROMOSOME_SIZE] = mandatory_count - unscheduled_mandatory;
//	chromosome[CHROMOSOME_SIZE + 1] = scheduled_optional_count + mandatory_count - unscheduled_mandatory;
//
//    free(unscheduled_mandatory_patients);
//    free(v_A);
//    free(v_B);
//    free(v_empty);
//}



//----------------------------------------------------ABOVE: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

//-----------------------------------------------------------BELOW: GENETIC ALGORITHM-----------------------------------------------------------------

//const int POPULATION_SIZE = 100, NUM_ITER = 10000, CONVERGENCE_STOPPING_CRITERION = 100;
//int** POPULATION, * G_BEST, CHROMOSOME_SIZE;
//int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
//int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, *chromosome;

//-------------------------------------------------------BELOW: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm2(PriorityQueue* pq);
void evaluateFitnessScore2(int* chromosome, PriorityQueue* pq);
extern void orderCrossover(void);
extern void swapMutation(void);
extern void generateNewChromosome(int chromo_num);
extern void generatePopulation(void);
//extern void crossoverTournamentSelection(void);
void crossoverTournamentSelection_temp2(void);
//void mutationTournamentSelection(void);
void mutationTournamentSelection_temp2(void);
void crossoverElitism2(void);
void mutationElitism2(void);
void initDataStructures2(void);
extern void freeDataStructures(void);
extern void printPopulation(void);

//-------------------------------------------------------ABOVE: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm2(PriorityQueue* pq)
{   /*
    In this implementation of GA: 
	1. We have a population of size POPULATION_SIZE.
	2. Each chromosome in the population is of size CHROMOSOME_SIZE.
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
    int i, j, best_fitness, g_best, best_fitness_mandatory = 0;
	//int fitness_storage_place[2]; // This array is used to store the 2 fitness scores of the chromosome. 
    unsigned int same_fitness_iter;
    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
    generatePopulation2();
    for (i = 0; i < POPULATION_SIZE; ++i)
        evaluateFitnessScore2(POPULATION[i], pq);
    //printf("%d\n", POPULATION[0]);

	///printPopulation();
	// GBEST: The best chromosome in the population. Currently - the first chromosome.
    memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE + 2) * sizeof(int));
    /*for (int i = 0; i < CHROMOSOME_SIZE; i++) {
        G_BEST[i] = POPULATION[0][i];
    }*/
    g_best = G_BEST[CHROMOSOME_SIZE+1];
    //printPopulation();

    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    for (i = 0; i <= NUM_ITER ; ++i) {
		if (best_fitness_mandatory == mandatory_count)
			break;
        if ((rand() / (float) RAND_MAX) <= p_c) {
            //crossoverTournamentSelection();
			crossoverTournamentSelection_temp2(); // Try the upper funciton as well as check which one yields better results.
            orderCrossover();
            //printf("\nCrossover Offsprings: ");

            // calculate the fitness of the 2 new offsprings
            for (j = 0; j < 2; ++j)
                evaluateFitnessScore2(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq);
            crossoverElitism2();
            for (j = 0; j < 2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE + 2));
        }
        else {
             //mutationTournamentSelection();
            mutationTournamentSelection_temp2(); // Try the upper funciton as well as check which one yields better results.
            /*printf("\nMutation Parent: ");
            for (int i = 0; i < CHROMOSOME_SIZE; i++) {
                printf("%d\t", MUTATE_PARENT_STORAGE_PLACE[i]);
            }*/
            swapMutation();
            // calculate the fitness of the new offspring
            evaluateFitnessScore2(MUTATED_OFFSPRING_STORAGE_PLACE, pq);
            mutationElitism2();
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 2));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        best_fitness = G_BEST[CHROMOSOME_SIZE+1];
        best_fitness_mandatory = G_BEST[CHROMOSOME_SIZE];
        /*if (best_fitness == g_best)
            same_fitness_iter++;
        else
            same_fitness_iter = 0;*/
        g_best = G_BEST[CHROMOSOME_SIZE+1];
    }

    //puts("\nChecking whether the iterations were over OR the algorithm converged: ");
    printf("Number of iterations: %d", i);
    // either i will be NUM_ITER (1000000) OR same_fitness_iter will be CONVERGENCE_STOPPING_CRITERION (100).
    // G_BEST is the best chromosome - use it for admitting patients.
}

void evaluateFitnessScore2(int* chromosome, PriorityQueue* pq)
{   // fitness score is the number of mandatory patients who were admitted in the scheduling period.
    // The best fitness score is CHROMOSOME_SIZE
    reset_values();
    admitPatientsGA2(&room_gender_map, pq, chromosome);
    reset_values();
    //empty_pq(pq);
}







void crossoverTournamentSelection_temp2(void)
{   // select 2 parents using Tournament Selection method
    int i, best_fitness, best_fitness_idx1 = 0, best_fitness_idx2 = 0;

    // select first parent
    best_fitness = POPULATION[0][CHROMOSOME_SIZE+1];
    for (i = 1; i < POPULATION_SIZE; ++i) {
        if (POPULATION[i][CHROMOSOME_SIZE+1] > best_fitness) {
            best_fitness = POPULATION[i][CHROMOSOME_SIZE+1];
            best_fitness_idx1 = i;
        }
    }

    // select second parent
    best_fitness = POPULATION[0][CHROMOSOME_SIZE+1];
    for (i = 1; i < POPULATION_SIZE; ++i) {
        if (POPULATION[i][CHROMOSOME_SIZE+1] > best_fitness && i != best_fitness_idx1) {
            best_fitness = POPULATION[i][CHROMOSOME_SIZE+1];
            best_fitness_idx2 = i;
        }
    }

    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[0], POPULATION[best_fitness_idx1], (CHROMOSOME_SIZE + 2) * sizeof(int));
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[1], POPULATION[best_fitness_idx2], (CHROMOSOME_SIZE + 2) * sizeof(int));
}

void mutationTournamentSelection_temp2(void)
{   // select 2 parents using Tournament Selection method
    int r1 = 0, r2 = 0, r3 = 0, f1 = 0, f2 = 0, f3 = 0;
    int best_fitness = 0, best_fitness_idx = 0, i;

    best_fitness = POPULATION[0][CHROMOSOME_SIZE+1];
    for (i = 1; i < POPULATION_SIZE; ++i) {
        if (POPULATION[i][CHROMOSOME_SIZE+1] > best_fitness) {
            best_fitness = POPULATION[i][CHROMOSOME_SIZE+1];
            best_fitness_idx = i;
        }
        else
            if (f3 > best_fitness) {
                best_fitness = f3;
                best_fitness_idx = r3;
            }
    }

    memcpy(MUTATE_PARENT_STORAGE_PLACE, POPULATION[best_fitness_idx], (CHROMOSOME_SIZE + 2) * sizeof(int));
}


void crossoverElitism2(void)
{
    int i, worst_fitness_chromosome_index = 0, second_worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE+1] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
            worst_fitness_chromosome_index = i;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE+1] < POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE+1] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE+1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
        memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE + 2));
    else {
        if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE+1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
            memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE + 2));
        return;
    }

    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE+1] > POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
        memcpy(POPULATION[second_worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE + 2));
}

void mutationElitism2(void)
{
    int i, worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE+1] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE+1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
        memcpy(POPULATION[worst_fitness_chromosome_index], MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 2));
}

void initDataStructures2(void)
{
    int i;

    // Allocate memory for single arrays
    MUTATED_OFFSPRING_STORAGE_PLACE = (int*)calloc(CHROMOSOME_SIZE + 2, sizeof(int));
    ASSERT(MUTATED_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for MUTATED_OFFSPRING_STORAGE_PLACE");

    MUTATE_PARENT_STORAGE_PLACE = (int*)calloc(CHROMOSOME_SIZE + 2, sizeof(int));
    ASSERT(MUTATE_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for MUTATE_PARENT_STORAGE_PLACE");

    G_BEST = (int*)calloc(CHROMOSOME_SIZE + 2, sizeof(int));
    ASSERT(G_BEST, "Dynamic Memory Allocation Error for G_BEST");

    // Allocate memory for crossover offspring storage
    CROSSOVER_OFFSPRING_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(CROSSOVER_OFFSPRING_STORAGE_PLACE, "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        CROSSOVER_OFFSPRING_STORAGE_PLACE[i] = (int*)calloc(CHROMOSOME_SIZE + 2, sizeof(int));
        ASSERT(CROSSOVER_OFFSPRING_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE[i]");
    }

    // Allocate memory for crossover parent storage
    CROSSOVER_PARENT_STORAGE_PLACE = (int**)calloc(2, sizeof(int*));
    ASSERT(CROSSOVER_PARENT_STORAGE_PLACE, "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE");

    for (i = 0; i < 2; ++i) {
        CROSSOVER_PARENT_STORAGE_PLACE[i] = (int*)calloc(CHROMOSOME_SIZE + 2, sizeof(int));
        ASSERT(CROSSOVER_PARENT_STORAGE_PLACE[i], "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE[i]");
    }

    POPULATION = (int**)calloc(POPULATION_SIZE, sizeof(int*));
    ASSERT(POPULATION, "Dynamic Memory Allocation Error for POPULATION");

    for (i = 0; i < POPULATION_SIZE; ++i) {
        POPULATION[i] = (int*)calloc(CHROMOSOME_SIZE + 2, sizeof(int));
        ASSERT(POPULATION[i], "Dynamic Memory Allocation Error for a CHROMOSOME");

        POPULATION[i][CHROMOSOME_SIZE] = -1;
        POPULATION[i][CHROMOSOME_SIZE+1] = -1;
    }
}


void generatePopulation2(void)
{
    int i, j, k;

    // Copy mandatory patients' IDs into the first chromosome followed by optional patients' IDs

    for (j = 0; j < mandatory_count; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;

    for (i = j; i < CHROMOSOME_SIZE; ++i)
        POPULATION[0][i] = optional_patients[i - j]->id;

    // Generate new chromosomes for the rest
    for (i = 1; i < POPULATION_SIZE; ++i)
        generateNewChromosome(i);
}

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

int main(void) {
    parse_json("data/instances/i08.json");
    PriorityQueue* pq;
    srand(0);
    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
    if (!pq)
        ASSERT(0, "Dynamic Memory Allocation Error for PQ");
	size = mandatory_count + optional_count;

    CHROMOSOME_SIZE = size;

    initDataStructures();
    initialize_room_gender_map(&room_gender_map);
    initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
    populate_room_gender_map(&room_gender_map);
    print_map(&room_gender_map);
    //initializePopulation();
    //generatePopulation();
    //printPopulation();
	printf("Mandatory Patients: %d\n", mandatory_count);
	printf("Optional Patients: %d\n", optional_count);
    reset_values();
    applyGeneticAlgorithm2(pq);
   // printf("Mandatory Patients: %d\n", size);
    reset_values();
    admitPatientsGA2(&room_gender_map, pq, G_BEST);
    printf("\nBest Fitness Score: %d\n", G_BEST[CHROMOSOME_SIZE]);
    printf("\nBest optional admitted: %d\n", G_BEST[CHROMOSOME_SIZE + 1]);

    //print_rooms();   
//    freeDataStructures();   
    create_dm_nurses_availability();
    sorting_nurse_id_max_load();
    create_3d_array();
    initialize_rooms_req(num_rooms);
    create_rooms_req();
    printf("\nBest Chromosome:");
    for (int i = 0; i <= CHROMOSOME_SIZE; i++)
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
    //free_nurses();
    //freeDataStructures();
    free(weights);

    return 0;
}