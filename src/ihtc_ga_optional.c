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

const int POPULATION_SIZE = 10000, NUM_ITER = 1000000, CONVERGENCE_STOPPING_CRITERION = 100;
int** POPULATION, * G_BEST, CHROMOSOME_SIZE, size;
int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, * chromosome;

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

void reset_Values();
void admit_optional_patients(int**, OTs**);


void empty_pq(PriorityQueue* pq) {
    HeapNode node;
    while (pq->current_size) {
        node = extractMaxFromPQ(pq);
    }
}

void update_LOS_of_patients_GA(int d, int* chromosome) {
    /* Check if any patient's length_of_stay is over and if yes then remove that patient from -
        1. the room he's been assigned to.
        2. any assignment in the NRA problem pertaining to this patient.
    */

    int i, g, r_id, admit_day, los, days_passed, gene_p_id;
    Node* p, * self;
    //GenderRoom* gen_array;
    Occupants occ;

    // first consider the occupants -
    // if an occupant's LOS is over then throw him out of the room and make space of a patient.
    for (i = 0; i < num_occupants; ++i) {
        occ = occupants[i];
        r_id = occ.room_id;
        g = occ.gen;
        days_passed = d - occ.length_of_stay;

        if (days_passed >= 0) {
            // throw the occupant out
            if (room[r_id].occupants_cap > 0)
                room[r_id].occupants_cap--;
            else
				ASSERT(0, "Occupants cap is already 0");
            if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated) {
                if (g == A) {
                    moveRoom(v_A, v_empty, r_id);
                }
                else if (g == B) {
                    moveRoom(v_B, v_empty, r_id);
                }
                else {
                    printf("Invalid gender value for room: %d\n", r_id);
                }

                room[r_id].gen = -1;
            }
        }
    }

    for (i = 0; i < CHROMOSOME_SIZE; ++i) {
        gene_p_id = chromosome[i];
        admit_day = patients[gene_p_id].admission_day;
        r_id = patients[gene_p_id].assigned_room_no;
        g = patients[gene_p_id].gen;
        los = patients[gene_p_id].length_of_stay;

        if (admit_day != -1) { // if true - means the patient has been admitted. 
            days_passed = d - admit_day;
            if (days_passed == los) {
                room[r_id].num_patients_allocated--;
                if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated) {
                    if (g == A) {
                        moveRoom(v_A, v_empty, r_id);
                    }
                    else if (g == B) {
                        moveRoom(v_B, v_empty, r_id);
                    }
                    else {
                        printf("Invalid gender value for room: %d\n", r_id);
                    }

                    room[r_id].gen = -1;
                }
            }
        }
    }
}

OTs* admitFromPQ_GA(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt, int* fitness_storage_place) {
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
            *node = extractMaxFromPQ(pq);  // Copy extracted node data
            p_id = node->patient_id;
            s_duration = patients[p_id].surgery_duration;

            while (current_ot_index < num_ots &&
                (current_ot->time_left[d] == 0 || current_ot->time_left[d] < s_duration)) {
                current_ot_index++;
                if (current_ot_index < num_ots) {
                    current_ot = ot_data_arr[current_ot_index];
                }
            }

            // If no valid OT was found, set a flag
            if (current_ot_index == num_ots) {
                flag = 1;
                current_ot_index--; // Ensures index stays in range for later use
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
                    surgeon[patients[p_id].surgeon_id].time_left[d] -= patients[p_id].surgery_duration;
                }
                else
                    pushback_Mand_Opt_PQ(vector, node);
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

int k = 0;

void reset_values() {
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
    }
    assign_occupants_to_rooms();

    //surgeon_structure
    for (int i = 0; i < num_surgeons; i++) {
        for (int j = 0; j < days; j++) {
            surgeon[i].time_left[j] = surgeon[i].max_surgery_time[j];
        }

    }

    //OT_structure
    for (int i = 0; i < num_ots; i++) {
        for (int j = 0; j < days; j++) {
            ot[i].time_left[j] = ot[i].max_ot_time[j];
        }
    }


}

int compare_release_Day(const void* a, const void* b) {
    Patient p_a = *(Patient*)a;
    Patient p_b = *(Patient*)b;

    // Compare max_ot_time for the global sorting day
    if (p_b.surgery_release_day != p_a.surgery_release_day) {
        return p_b.surgery_release_day - p_a.surgery_release_day; // Descending order
    }
    return p_a.id - p_b.id; // Secondary sort by id (ascending order)
}

void admitPatientsGA(int** room_gender_map, PriorityQueue* pq, int* chromosome) {
    int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, day, p_counter;
    int unscheduled_mandatory = 0, scheduled_optional_count = 0, assigned_ot, flag, max = 0;
    OTs** ot_data_arr, * current_ot;

    v_A = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_B = (RoomVector*)calloc(1, sizeof(RoomVector));
    v_empty = (RoomVector*)calloc(1, sizeof(RoomVector));

    make_3_vectors(room_gender_map);
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
                
                // for optional patients
                if (patients[p_id].surgery_due_day == -1) {
					if (patients[p_id].surgery_release_day <= day) {
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
								//insertNodeInPQ(pq, makeHeapNode(p_id, 0, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
								continue;
							}
						}
						else {
							assigned_ot = current_ot->id;
						}
						// if the control comes here - that means surgeon and OT are available and only room remains
						// look for a suitable room
						r_id = (patients[p_id].gen) ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
						if (r_id != -1 && surgeon[patients[p_id].surgeon_id].time_left[day] >= patients[p_id].surgery_duration) {
							room[r_id].num_patients_allocated++;
							patients[p_id].admission_day = day;
							patients[p_id].assigned_ot = assigned_ot;
							patients[p_id].assigned_room_no = r_id;
							patients[p_id].is_admitted = 1;
							surgeon[patients[p_id].surgeon_id].time_left[day] -= patients[p_id].surgery_duration;
							ot[assigned_ot].time_left[day] -= patients[p_id].surgery_duration;

							scheduled_optional_patients[p_id] = 1;
                            scheduled_optional_count++;
						}
                        else {
                            r_id = -1;
							assigned_ot = -1;
                            patients[p_id].admission_day = -1;
                            continue;
                        }
					}
					continue;
				}
				// for mandatory patients

                if (patients[p_id].surgery_due_day < day && patients[p_id].admission_day == -1 && unscheduled_mandatory_patients[p_id] == 0) {
                    ++unscheduled_mandatory;
                    unscheduled_mandatory_patients[p_id] = 1;
                    continue;
                }
                else {
                    if (patients[p_id].surgery_release_day <= day && patients[p_id].surgery_due_day >= day) {
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

                        }
                        else {
                            r_id = -1;
                            assigned_ot = -1;
                            patients[p_id].admission_day = -1;
                            continue;
                        }
                    }
                }
            }
        }
    }
    //print_surgeons(surgeon);
    //print_ots(ot);
    //print_rooms();
   // printVector("Empty", v_empty);

    for (int i = 0; i < max+1; i++) {
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

    /*printf("\nunscheduled_mandatory: \n");
    for (int i = 0; i < CHROMOSOME_SIZE; i++) {
        if (unscheduled_mandatory_patients[i] == 1) {
            printf("%d\t", i);
        }
    }*/
    // admit_optional_patients(&room_gender_map, ot_data_arr);
    free(unscheduled_mandatory_patients);
    free(v_A);
    free(v_B);
    free(v_empty);
}

/*
void admit_optional_patients(int** room_gender_map, OTs** ot_data_arr, int* fitness_storage_place) {
	// fitness_storage_place[0] : unscheduled_mandatory
	// fitness_storage_place[1] : total_unscheduled, i.e. unscheduled_mandatory + unscheduled_optional
    int p_id, r_id, day, assigned_ot, flag, j, i, scheduled_optional=0;
    OTs* current_ot;

    for (day = 0; day < days; day++) {
		sort_ot_data_arr(ot_data_arr, day);
        current_ot = ot_data_arr[0];

        for (int i = 0; i < optional_count; i++) {
            p_id = optional_patients[i]->id;
            if (patients[p_id].surgery_release_day > day || patients[p_id].admission_day != -1) {
                continue;
            }
            else {
                //if (patients[p_id].surgery_due_day < day && patients[p_id].admission_day == -1) {
                //    continue;
                //}
                // else {
                if (patients[p_id].surgery_release_day <= day) {
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
                        scheduled_optional++;
                    }
                    else {
						assigned_ot = -1;
                        r_id = -1;
                        continue;
                    }
                }
            }
        }
    }
}
*/

//----------------------------------------------------ABOVE: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

//-----------------------------------------------------------BELOW: GENETIC ALGORITHM-----------------------------------------------------------------

//const int POPULATION_SIZE = 100, NUM_ITER = 10000, CONVERGENCE_STOPPING_CRITERION = 100;
//int** POPULATION, * G_BEST, CHROMOSOME_SIZE;
//int** CROSSOVER_OFFSPRING_STORAGE_PLACE, ** CROSSOVER_PARENT_STORAGE_PLACE;
//int* MUTATED_OFFSPRING_STORAGE_PLACE, * MUTATE_PARENT_STORAGE_PLACE, *chromosome;

//-------------------------------------------------------BELOW: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm(PriorityQueue* pq);
void evaluateFitnessScore(int* chromosome, PriorityQueue* pq);
void orderCrossover(void);
void swapMutation(void);
void generateNewChromosome(int chromo_num);
void generatePopulation(void);
void crossoverTournamentSelection(void);
void crossoverTournamentSelection_temp(void);
void mutationTournamentSelection(void);
void mutationTournamentSelection_temp(void);
void crossoverElitism(void);
void mutationElitism(void);
void initDataStructures(void);
void freeDataStructures(void);
void printPopulation(void);

//-------------------------------------------------------ABOVE: FUNCTION DEFINITIONS FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm(PriorityQueue* pq)
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
    int i, j, best_fitness, g_best;
	//int fitness_storage_place[2]; // This array is used to store the 2 fitness scores of the chromosome. 
    unsigned int same_fitness_iter;
    float p_c = 0.85; // try with 0.6, 0.7, 0.8, 0.9
    generatePopulation();
    for (i = 0; i < POPULATION_SIZE; ++i)
        evaluateFitnessScore(POPULATION[i], pq);
    //printf("%d\n", POPULATION[0]);

	printPopulation();
	// GBEST: The best chromosome in the population. Currently - the first chromosome.
    memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE + 2) * sizeof(int));
    /*for (int i = 0; i < CHROMOSOME_SIZE; i++) {
        G_BEST[i] = POPULATION[0][i];
    }*/
    g_best = G_BEST[CHROMOSOME_SIZE+1];
    //printPopulation();

    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    for (i = 0; i <= NUM_ITER; ++i) {
        if ((rand() / (float) RAND_MAX) <= p_c) {
            //crossoverTournamentSelection();
			crossoverTournamentSelection_temp(); // Try the upper funciton as well as check which one yields better results.
            orderCrossover();
            //printf("\nCrossover Offsprings: ");

            // calculate the fitness of the 2 new offsprings
            for (j = 0; j < 2; ++j)
                evaluateFitnessScore(CROSSOVER_OFFSPRING_STORAGE_PLACE[j], pq);
            crossoverElitism();
            for (j = 0; j < 2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE+1] > G_BEST[CHROMOSOME_SIZE+1])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE + 2));
        }
        else {
             //mutationTournamentSelection();
            mutationTournamentSelection_temp(); // Try the upper funciton as well as check which one yields better results.
            /*printf("\nMutation Parent: ");
            for (int i = 0; i < CHROMOSOME_SIZE; i++) {
                printf("%d\t", MUTATE_PARENT_STORAGE_PLACE[i]);
            }*/
            swapMutation();
            // calculate the fitness of the new offspring
            evaluateFitnessScore(MUTATED_OFFSPRING_STORAGE_PLACE, pq);
            mutationElitism();
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE+1] > G_BEST[CHROMOSOME_SIZE+1])
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 2));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        best_fitness = G_BEST[CHROMOSOME_SIZE+1];
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

void evaluateFitnessScore(int* chromosome, PriorityQueue* pq)
{   // fitness score is the number of mandatory patients who were admitted in the scheduling period.
    // The best fitness score is CHROMOSOME_SIZE
    reset_values();
    admitPatientsGA(&room_gender_map, pq, chromosome);
    reset_values();
    //empty_pq(pq);
}

void orderCrossover(void) {
    // This function may produce errors so we'll have to recheck in case it yields violations.
    int r1, r2, k, m, i, index;
    int gene1, gene2, max = 0, temp;
    for (i = 0; i < CHROMOSOME_SIZE; i++) {
        if (max < CROSSOVER_PARENT_STORAGE_PLACE[0][i])
            max = CROSSOVER_PARENT_STORAGE_PLACE[0][i];
    }

    // Dynamic allocation for visited check
    int* visited1 = (int*)calloc(max + 1, sizeof(int));
    int* visited2 = (int*)calloc(max + 1, sizeof(int));

    if (!visited1 || !visited2) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    // Select two random crossover points
    do {
        r1 = rand() % CHROMOSOME_SIZE;
        r2 = rand() % CHROMOSOME_SIZE;
    } while (abs(r1 - r2) < 2);

    if (r1 > r2) {
        temp = r1;
        r1 = r2;
        r2 = temp;
    }

    // Initialize offspring with -1 to identify unassigned positions
    for (i = 0; i < CHROMOSOME_SIZE; i++) {
        CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = -1;
        CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = -1;
    }

    // Step 1: Copy the segment from r1 to r2 and mark visited elements **safely**
    for (i = r1; i <= r2; i++) {
        gene1 = CROSSOVER_PARENT_STORAGE_PLACE[0][i];
        gene2 = CROSSOVER_PARENT_STORAGE_PLACE[1][i];

        CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = gene1;
        CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = gene2;

        if (gene1 >= 0)
            visited1[gene1] = 1;

        if (gene2 >= 0)
            visited2[gene2] = 1;
    }

    // Step 2: Fill remaining positions while maintaining order
    k = (r2 + 1) % CHROMOSOME_SIZE;
    m = k;

    for (i = 0; i < CHROMOSOME_SIZE; i++) {
        index = (r2 + 1 + i) % CHROMOSOME_SIZE;

        gene1 = CROSSOVER_PARENT_STORAGE_PLACE[0][index];
        if (gene1 >= 0 && !visited1[gene1]) {
            while (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][k] != -1) {
                k = (k + 1) % CHROMOSOME_SIZE;
            }
            CROSSOVER_OFFSPRING_STORAGE_PLACE[1][k] = gene1;
            visited2[gene1] = 1;
        }

        gene2 = CROSSOVER_PARENT_STORAGE_PLACE[1][index];
        if (gene2 >= 0 && !visited2[gene2]) {
            while (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][m] != -1) {
                m = (m + 1) % CHROMOSOME_SIZE;
            }
            CROSSOVER_OFFSPRING_STORAGE_PLACE[0][m] = gene2;
            visited1[gene2] = 1;
        }
    }

    free(visited1);
    free(visited2);
}

void swapMutation(void)
{   // take the offspring from MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2;
    for (int i = 0; i < CHROMOSOME_SIZE; i++) {
        MUTATED_OFFSPRING_STORAGE_PLACE[i] = MUTATE_PARENT_STORAGE_PLACE[i];
    }

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
    int i, j, k;

	// Copy mandatory patients' IDs into the first chromosome followed by optional patients' IDs

    for (j = 0; j < mandatory_count; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;
            
    for (i = j; i < CHROMOSOME_SIZE; ++i)
        POPULATION[0][i] = optional_patients[i-j]->id;

    // Generate new chromosomes for the rest
    for (i = 1; i < POPULATION_SIZE; ++i)
        generateNewChromosome(i);
}

//void crossoverTournamentSelection(void)
//{   // select 2 parents using Tournament Selection method
//    int r11, r12, r13, r21, r22, r23, f11, f12, f13, f21, f22, f23;
//    int best_fitness, best_fitness_idx1, best_fitness_idx2;
//
//    // select first parent
//    do {
//        r11 = rand() % (POPULATION_SIZE);
//        r12 = rand() % (POPULATION_SIZE);
//        r13 = rand() % (POPULATION_SIZE);
//    } while (r11 == r12 || r12 == r13 || r11 == r13);
//
//    // select the chromosome1 with the best fitness
//    f11 = POPULATION[r11][CHROMOSOME_SIZE+1];
//    f12 = POPULATION[r12][CHROMOSOME_SIZE+1];
//    f13 = POPULATION[r13][CHROMOSOME_SIZE+1];
//    best_fitness = f11;
//    best_fitness_idx1 = r11;
//
//    if (f12 > best_fitness) {
//        if (f13 > f12) {
//            best_fitness = f13;
//            best_fitness_idx1 = r13;
//        }
//        else {
//            best_fitness = f12;
//            best_fitness_idx1 = r12;
//        }
//    }
//    else
//        if (f13 > best_fitness) {
//            best_fitness = f13;
//            best_fitness_idx1 = r13;
//        }
//
//
//    // select second parent
//    do {
//        r21 = rand() % (POPULATION_SIZE);
//        r22 = rand() % (POPULATION_SIZE);
//        r23 = rand() % (POPULATION_SIZE);
//    } while (r21 == r22 || r22 == r23 || r21 == r23 ||
//        r11 == r21 || r11 == r22 || r11 == r23 ||
//        r12 == r21 || r12 == r22 || r12 == r23 ||
//        r13 == r21 || r13 == r22 || r13 == r23);
//
//    // select the chromosome2 with the best fitness
//    f21 = POPULATION[r21][CHROMOSOME_SIZE+1];
//    f22 = POPULATION[r22][CHROMOSOME_SIZE+1];
//    f23 = POPULATION[r23][CHROMOSOME_SIZE+1];
//    best_fitness = f21;
//    best_fitness_idx2 = r21;
//
//    if (f22 > best_fitness) {
//        if (f23 > f22) {
//            best_fitness = f23;
//            best_fitness_idx2 = r23;
//        }
//        else {
//            best_fitness = f22;
//            best_fitness_idx2 = r22;
//        }
//    }
//    else
//        if (f23 > best_fitness) {
//            best_fitness = f23;
//            best_fitness_idx2 = r23;
//        }
//    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[0], POPULATION[best_fitness_idx1], (CHROMOSOME_SIZE + 2) * sizeof(int));
//    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[1], POPULATION[best_fitness_idx2], (CHROMOSOME_SIZE + 2) * sizeof(int));
//}
//
//void mutationTournamentSelection(void)
//{   // select 2 parents using Tournament Selection method
//    int r1, r2, r3, f1, f2, f3;
//    int best_fitness, best_fitness_idx;
//
//    // select first parent
//    do {
//        r1 = rand() % (POPULATION_SIZE);
//        r2 = rand() % (POPULATION_SIZE);
//        r3 = rand() % (POPULATION_SIZE);
//    } while (r1 == r2 || r2 == r3 || r1 == r3);
//
//    // select the chromosome1 with the best fitness
//    f1 = POPULATION[r1][CHROMOSOME_SIZE+1];
//    f2 = POPULATION[r2][CHROMOSOME_SIZE+1];
//    f3 = POPULATION[r3][CHROMOSOME_SIZE+1];
//    best_fitness = f1;
//    best_fitness_idx = r1;
//
//    if (f2 > best_fitness) {
//        if (f3 > f2) {
//            best_fitness = f3;
//            best_fitness_idx = r3;
//        }
//        else {
//            best_fitness = f2;
//            best_fitness_idx = r2;
//        }
//    }
//    else
//        if (f3 > best_fitness) {
//            best_fitness = f3;
//            best_fitness_idx = r3;
//        }
//
//    memcpy(MUTATE_PARENT_STORAGE_PLACE, POPULATION[best_fitness_idx], (CHROMOSOME_SIZE + 2) * sizeof(int));
//}

void crossoverTournamentSelection_temp(void)
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

void mutationTournamentSelection_temp(void)
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


void crossoverElitism(void)
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

void mutationElitism(void)
{
    int i, worst_fitness_chromosome_index = 0;

    for (i = 1; i < POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE+1] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE+1] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE+1])
        memcpy(POPULATION[worst_fitness_chromosome_index], MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 2));
}

void initDataStructures(void)
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

void freeDataStructures(void)
{
    int i;
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

void printPopulation(void)
{
    int i, j;
    for (i = 0; i < POPULATION_SIZE; ++i) {
        printf("Chromosome %d: ", i + 1);
        for (j = 0; j < CHROMOSOME_SIZE + 2; ++j) {
			if (j == CHROMOSOME_SIZE)
				printf("\nMandatory Patients Admitted: %d", POPULATION[i][j]);
			else if (j == CHROMOSOME_SIZE + 1)
				printf("\nFitness Score: %d", POPULATION[i][j]);
			else
                printf("%d ", POPULATION[i][j]);
        }
        putchar('\n');
    }
}

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

int main(void) {
    parse_json("data/instances/i27.json");
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
    reset_values();
    applyGeneticAlgorithm(pq);
    printf("Mandatory Patients: %d\n", size);
    printf("Best Fitness Score: %d\n", G_BEST[CHROMOSOME_SIZE]);
    reset_values();
    //admitPatientsGA(&room_gender_map, pq, G_BEST);

    //print_rooms();   
//    freeDataStructures();   
    create_dm_nurses_availability();
    sorting_nurse_id_max_load();
    create_3d_array();
    initialize_rooms_req(num_rooms);
    create_rooms_req();
    /*printf("\nBest Chromosome:");
    for (int i = 0; i <= CHROMOSOME_SIZE; i++)
        printf("%d\t", G_BEST[i]);*/
    printf("Mandatory Patients: %d\n", size);
    printf("Best Fitness Score: %d\n", G_BEST[CHROMOSOME_SIZE]);
    nurse_assignments();
    create_json_file(patients, num_patients, nurses, num_nurses, num_rooms, "i27", "D:/major_code/build/output");

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