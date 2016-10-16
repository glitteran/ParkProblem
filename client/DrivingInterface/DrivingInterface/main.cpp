
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

float C;  //차선의 위치 및 차량의 차선 변경 등의 케이스에 따라 유동적인 값
float C2;	//최적 속도를 구할 때 운전 조건하에 따른 교정계수
float D; //최적 속도를 구할 때 운전 조건하에 따른 교정계수
float V; //앞차와 거리를 고려한 최적 속도
float Vmax = 100;	//최대속도


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
	//steerCmd 핸들 조작시 움직여야 할 각도 [-1 1]
	//accelCmd 속도 상승이 필요할 때 조절 수치 [0 1]
	//brakeCmd 속도 하락이 필요할 때 조절 수치 [0 1]
	//backwardCmd 전진/후진 [0이면 전진 0이아니면 후진]

	//speed 현재주행속도
	//damage 현재 차량의 누적 데미지 점수
	//damage_max 허용 데미지 최대치
	//my_rank 전체 차량 중 나의 순위
	//total_car_num 형재 주행 중인 전체 차량 수
	//opponent_rank 상대편 순위

	/**차량정보**/
	//dist_cars 전후방차량,각차량의 toMiddle값 전후방 100미터이내
	//dist_cars[0,2,4,6,8] 전방 차량과의 거리 distcars[1,3,5,7,9] 각 전방 차량의 toMiddle값
	//dist_cars[10,12,14,16,18] 후방 차량과의 거리 distcars[11,13,15,17,19] 각 후방 차량의 toMiddle값
	//주변 차량을 인지하고 충돌 예측 및 회피를 판단해라
	//차량 중심 가운데를 영점으로 왼쪽은 음수(-), 오른쪽은 양수(+)로 미터단위
	
	/**트랙정보**/
	//8~10미터 길이를 가지는 구간으로 제공 [-3.14 3.14]
	//전방 20개 구간에 대한 정보(위치와 각도)
	//track_current_angle 도로 방향과 지도 중심선 사이의 파이값 : 현재 트랙의 각도 정보
	//track_angles[20] 전방 20개 트랙의 각도정보
	//track_dists[20] 시작점에서 부터 각 트랙 중심점까지의 거리
	//angle 차량의 정면과 도로 방향 사이의 파이값 [-3.14 좌측으로 회전 3.14 그 반대]
	//toMiddle 차량 중심에서 도로 전체 중심까지의 거리
	//track_width 트랙의 폭
	//toMiddle,track_width는 도로상에서 현재 위치를 인지하는데 활용. 트랙 이탈시 차선 복귀, 차선 변경을 위해 사용
	//toStart : 출발점에서 현재 차량의 위치까지 거리
	//dist_track 전체 트랙의 길이(1lap기준)
	//track_dist_straight 차량 전방의 연속된 직진 트랙의 길이
	//track_curve_type 차량 전방의 직진이 끝나는 지점의 커브 종류 
	//CURVE_TYPE_RIGHT(1)  우회전코스 CURVE_TYPE_LEFT (2) 좌회전코스
	//track_dist_straight, track_curve_type 트랙 유형파악하고 주행 전략을 수립할 때 사용

	
	C = 1;	//차선의 위치 및 차량의 차선 변경 등의 케이스에 따라 유동적인 값
	V = Vmax * (1 - exp(-C2 / Vmax*shared->dist_cars[0] - D));

	

	//TO-DO : 알고리즘을 작성하세요.

	//Output : 4개의 Output Cmd 값을 도출하세요.
	//shared->steerCmd = 0.0;
	shared->steerCmd = C * (shared->angle - shared->toMiddle / shared->track_width);
	shared->accelCmd = 1;
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
