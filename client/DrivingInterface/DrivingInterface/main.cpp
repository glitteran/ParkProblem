
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <math.h>

#include <Windows.h>

#define SHARED_MOMORY_NAME1 "TORCS_SHARED1"
#define SHARED_MOMORY_NAME2 "TORCS_SHARED2"

#define CURVE_TYPE_RIGHT		1
#define CURVE_TYPE_LEFT			2
#define CURVE_TYPE_STRAIGHT		3

#define GEAR_FORWARD			0   // 전진 (D)
#define GEAR_BACKWARD			-1	// 후진 (R)

#define INPUT_AICAR_SIZE			20
#define INPUT_FORWARD_TRACK_SIZE	20

struct shared_use_st
{
	// System Value
	BOOL connected;
	int written;

	// Driving Parameters
	double toMiddle;
	double angle;
	double speed;

	// Track Parameters
	double toStart;
	double dist_track;
	double track_width;
	double track_dist_straight;
	int    track_curve_type;
	double track_forward_angles[INPUT_FORWARD_TRACK_SIZE];
	double track_forward_dists[INPUT_FORWARD_TRACK_SIZE];
	double track_current_angle;

	// Other Cars Parameters
	double dist_cars[INPUT_AICAR_SIZE];

	// Racing Info. Parameters
	double damage;
	double damage_max;
	int    total_car_num;
	int    my_rank;
	int    opponent_rank;

	// Output Values
	double steerCmd;
	double accelCmd;
	double brakeCmd;
	int    backwardCmd;
};

int controlDriving(shared_use_st *shared){
	if (shared == NULL) return -1;

	//Input : shared_user_st

	//TO-DO : 알고리즘을 작성하세요.

	//Output : 4개의 Output Cmd 값을 도출하세요.
	shared->steerCmd = 0.0;
	shared->accelCmd = 0.2;
	shared->brakeCmd = 0.0;
	shared->backwardCmd = GEAR_FORWARD;

	return 0;
}

void endShare(struct shared_use_st *&shared, HANDLE &hMap){
	// shared memory initialize
	if (shared != NULL)	{
		UnmapViewOfFile(shared);
		shared = NULL;
	}
	if (hMap != NULL) {
		CloseHandle(hMap);
		hMap = NULL;
	}
}

int main(int argc, char **argv){
	////////////////////// set up memory sharing
	struct shared_use_st *shared = NULL;

	// try to connect to shared memory 1
	HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, SHARED_MOMORY_NAME1);
	if (hMap == NULL){
		fprintf(stderr, "Shared Memory Map open failed.\n");
		exit(EXIT_FAILURE);
	}

	shared = (struct shared_use_st*) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(struct shared_use_st));
	if (shared == NULL){
		fprintf(stderr, "Shared Memory Map open failed.\n");
		exit(EXIT_FAILURE);
	}

	// shared memory 1 is already occupied.
	if (shared->connected == true) {
		endShare(shared, hMap);
		hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, SHARED_MOMORY_NAME2);
		if (hMap == NULL){
			fprintf(stderr, "Shared Memory Map open failed.\n");
			exit(EXIT_FAILURE);
		}
		shared = (struct shared_use_st*) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(struct shared_use_st));
		if (shared == NULL){
			fprintf(stderr, "Shared Memory Map open failed.\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("\n********** Memory sharing started, attached at %X **********\n", shared);

	////////////////////// DON'T TOUCH IT - Default Setting
	shared->connected = true;
	shared->written = 0;
	////////////////////// END Default Setting

	////////////////////// Initialize
	shared->steerCmd = 0.0;
	shared->accelCmd = 0.0;
	shared->brakeCmd = 0.0;
	shared->backwardCmd = GEAR_FORWARD;
	////////////////////// END Initialize

	while (shared->connected){
		if (shared->written == 1) { // the new image data is ready to be read
			controlDriving(shared);
			shared->written = 0;
		}

		if (_kbhit()){
			char key = _getch();
			if (key == 'q' || key == 'Q'){
				break;
			}
		}
	}

	endShare(shared, hMap);

	return 0;
}
