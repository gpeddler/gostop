#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum {false, true} bool;

typedef enum{
	Fee, Kwang, Band_r, Band_b, Band_c, Band_x, Animal, Animal_b, Fee_d, Kwang_x
}ENUM_INDEX;

typedef struct {
	unsigned month : 8;
	ENUM_INDEX index : 4;
}Card;

typedef struct _List{
	Card data;
	struct _List* prev;
	struct _List* next;
}List;

typedef struct {
	List* card;

	List* point_stack;

	int Fee_count;
	int Kwang_count;
	int Band_r_count;
	int Band_b_count;
	int Band_c_count;
	int Band_x_count;
	int Animal_count;
	int Animal_b_count;
	int Fee_d_count;
	int Kwang_x_count;//비광 추가됨

	int go;
	int shake;

	int point;
	int money;

	int num_sulsa;
	bool cont_sulsa;

}Player;

void _cardDataMapping(Card*, Card*);
void moveFromTo(List**, List**);
void sortList(List**);
void switchList(List**, List**);
void submitCard(List**, Player**, int, List**);
void clear(List**);
void applyChangeList(void*, List**);
List* findFee(List**);
List* findCard(List**, List**);
List* setDeck(Card*);
List* addNode(List*, Card);

void cal_money(Player **);
void cal_point(Player **);
void get_count(Player **, int, int *);
void cal_card_count(Player **);

void init(List**, List**, Player**);
void update(List**, List**, Player**);
void draw(Player**, List*);

int resultChongTong(Player**);
int getListNum(List*);
void printCardIndexName(ENUM_INDEX);

void saveFile(List**, List**, Player**);
void loadFile(List**, List**, Player**);

char name[] = {'A', 'B', 'C'};
int turn = 0;
int start_turn = 0;
int cycle = 0;

int pay = 100;

Card allCard[] = {{1, Kwang}, {1, Band_r}, {1, Fee}, {1, Fee},
				{2, Animal_b}, {2, Band_r}, {2, Fee}, {2, Fee},
				{3, Kwang}, {3, Band_r}, {3, Fee}, {3, Fee},
				{4, Animal_b}, {4, Band_c}, {4, Fee}, {4, Fee},
				{5, Animal}, {5, Band_c}, {5, Fee}, {5, Fee},
				{6, Animal}, {6, Band_b}, {6, Fee}, {6, Fee},
				{7, Animal}, {7, Band_c}, {7, Fee}, {7, Fee},
				{8, Kwang}, {8, Animal_b}, {8, Fee}, {8, Fee},
				{9, Fee_d}, {9, Band_b}, {9, Fee}, {9, Fee},
				{10, Animal}, {10, Band_b}, {10, Fee}, {10, Fee},
				{11, Kwang}, {11, Fee_d}, {11, Fee}, {11, Fee},
				{12, Kwang_x}, {12, Animal}, {12, Band_x}, {12, Fee_d}};

int main(void){
	srand((unsigned)time(NULL));

	List* deckHead = NULL;
	List* groundHead = NULL;

	int i;
	Player* player[3];
	for(i = 0; i < 3; i++){
		player[i] = (Player*)malloc(sizeof(Player));
		player[i]->card = NULL;
		player[i]->point_stack = NULL;
		player[i]->money = 100000;
	}
	
	init(&deckHead, &groundHead, player);
	while(1){
		for(i = 0; i < 3; i++){
			if(player[i]->money <= 0){
				printf("Player%c 파산", name[i]);
				break;
			}
		}
		draw(player, groundHead);
		update(&deckHead, &groundHead, player);
	}

	return 0;
}

void clear(List** list){
	List* tmp = *list;
	*list = NULL;

	if(tmp != NULL){
		for(; tmp;){
			List *c = tmp;
			tmp = tmp->next;
			free (c);
		}
	}
}

void init(List** deck, List** ground, Player** player){
	int i, j;

	clear(deck);
	clear(ground);

	(*deck) = setDeck(allCard);

	for(i = 0; i < 3; i++){
		clear(&player[i]->card);
		clear(&player[i]->point_stack);
		player[i]->point = 0;
		player[i]->num_sulsa = 0;
		player[i]->cont_sulsa = false;
		player[i]->go = 0;
		player[i]->shake = 0;
	}

	for(i = 0; i < 6; i++){
		moveFromTo(deck, ground);
	}


	for(i = 0; i < 3; i++){
		for(j = 0; j < 7; j++){
			moveFromTo(deck, &player[i]->card);
		}
	}

	cycle = 0;
	start_turn = turn;

	//총통
	int result = resultChongTong(player);
	if(result != -1){
		printf("[게임종료] Player%c 총통입니다", name[result]);
		turn = result;
		pay = 100;
		int command = getchar();
		init(deck, ground, player);
	}
}

void update(List** deck, List** ground, Player** player){
	printf("command : ");

	/* 커멘드 입력에 따른 행동 */
	char command[10];
	while((scanf("%s", command)) != EOF){
		if(strcmp("h", command) == 0 || strcmp("help", command) == 0){ // 헬프
			printf("g(o): 고, s(top): 스톱, e(exit): 끝내기, b(alance): 잔고, 1~7: 선택, 9: 십끗, save: 저장, load: 불러오기\n");
			printf("command : ");
		}else if(command[0] >= 49 && command[0] <=55){ // 1~7 숫자입력 (카드선택)
			if(getListNum(player[turn]->card) >= command[0] - 48){
				submitCard(ground, player, command[0] - 48, deck);
				break;
			}else{
				printf("Card Not found\n");
				printf("command : ");
			}
		}else if(strcmp("b", command) == 0 || strcmp("balance", command) == 0){ // 잔고확인
			printf("PlayerA : %d\n", player[0]->money);
			printf("PlayerB : %d\n", player[1]->money);
			printf("PlayerC : %d\n\n", player[2]->money);
			printf("command : ");
		}else if(strcmp("save", command) == 0){ // 게임저장
			saveFile(deck, ground, player);
			printf("game save\n");
			printf("command : ");
		}else if(strcmp("load", command) == 0){ // 게임로드
			loadFile(deck, ground, player);
			break;
		}else if(strcmp("exit", command) == 0){ // 게임종료
			exit(0);
		}
	}

	/* 점수 총계 계산 */
	cal_card_count(player);
	cal_point(player);

	int c; //command
	bool gameEnd = false;

	/* 점수가 났는지 확인 */
	if(player[turn]->point >= 3){
		draw(player, (*ground));

		if(player[turn]->card == NULL){
			gameEnd = true;
		}else{
			printf("GO? STOP? (1 or 2) : ");
			while((c = getchar()) != EOF){
				if(c == 49){
					player[turn]->go++;
					break;
				}else if(c == 50){
					gameEnd = true;
					break;
				}
			}
		}
	}

	/* 점수가 난 이후 플레이어가 스탑 or 게임종료 */
	if(gameEnd){
		printf("player%c win\nContinue? Exit? (1 or Ctrl + C)", name[turn]);
		while((c = getchar()) != EOF){
			if(c == 49){
				pay = 100;
				cal_money(player); // 정산
				init(deck, ground, player); // 초기화
				break;
			}
		}
	}
	
	/* 나가리가 확인 될 경우 게임 종료 */
	if(getListNum(player[0]->card) + getListNum(player[1]->card) + getListNum(player[2]->card) <= 0){
		printf("NAGARI..\nContinue? Exit? (1 or Ctrl + C)");
		while((c = getchar()) != EOF){
			if(c == 49){
				init(deck, ground, player); // 초기화
				pay *= 2;
				break;
			}
		}
	}

	/* 턴 순환 */
	turn++;
	if(turn >= 3){
		turn = 0;
	}

	/* 현재 사이클 계산 */
	if(start_turn == turn){
		cycle++;
	}
}

void draw(Player** player, List* ground){

	/* 출력 리스트 정렬 */
	sortList(&ground);
	sortList(&player[0]->card);
	sortList(&player[1]->card);
	sortList(&player[2]->card);
	sortList(&player[0]->point_stack);
	sortList(&player[1]->point_stack);
	sortList(&player[2]->point_stack);

	system("clear"); // 터미널 클리어

	int i;
	for(i = 0; i < 3; i++){
		/* 순서 + 플레이어 이름 표시 */
		if(turn == i){
			printf("*");
		}
		printf("%c  : ", name[i]);

		/* 유저의 소유 카드 출력 */
		List* listp = NULL;
		for(listp = player[i]->card; listp; listp = listp->next){
			printf("%d", listp->data.month);
			printCardIndexName(listp->data.index);
			printf(" ");
		}

		/* 유저의 점수 출력 */
		printf("\n(POINT:%d) : ", player[i]->point);

		/* 유저의 점수 카드 출력 */
		for(listp = player[i]->point_stack; listp; listp = listp->next){
			printf("%d", listp->data.month);
			printCardIndexName(listp->data.index);
			printf(" ");
		}

		printf("\n\n");	
	}

	/* 바닥에 놓인 카드 출력 */
	List* listp = NULL;
	printf("Ground : ");
	for(listp = ground; listp; listp = listp->next){
		printf("%d", listp->data.month);
		printCardIndexName(listp->data.index);
		printf(" ");
	}
	printf("\n\n");
}

void printCardIndexName(ENUM_INDEX index){
	/* 카드 이름 출력 */
	if(index == Fee){
		printf("피_");
	}else if(index == Kwang){
		printf("광_");
	}else if(index == Band_r){
		printf("홍단");
	}else if(index == Band_b){
		printf("청단");
	}else if(index == Band_c){
		printf("초단");
	}else if(index == Band_x){
		printf("띠_");
	}else if(index == Animal){
		printf("끗_");
	}else if(index == Animal_b){
		printf("끗새");
	}else if(index == Fee_d){
		printf("쌍피");
	}else if(index == Kwang_x){
		printf("비광");
	}
}


/* ---- Game Rule ---- */
int resultChongTong(Player** player){
	/* 총통 계산 */
	int i;
	for(i = 0; i < 3; i++){
		int num_card = 0;
		int now_card = 0;
		int num_sameCard = 0;

		/* 사용자의 총 카드 수 체크 (4장 이상) */
		List* tmp;
		for(tmp = player[i]->card; tmp; tmp = tmp->next){
			num_card ++;
		}

		/* 사용자의 카드 중 4장의 '월'이 같은지 비교 */
		if(num_card > 4){
			for(tmp = player[i]->card; tmp; tmp = tmp->next){
				if(now_card != tmp->data.month){
					now_card = tmp->data.month;
					num_sameCard = 0;
				}else{
					num_sameCard++;
				}
				if(num_sameCard >= 3){
					return i;
				}
			}
		}
	}
	return -1;
}


/* ---- Managing List ---- */

int getListNum(List* list){
	/* 리스트의 개수 리턴 */
	int i = 0;
	List* tmp;
	for(tmp = list; tmp; tmp=tmp->next){
		i++;

		if(i > 100){
			break;
		}
	}

	return i;
}

void _cardDataMapping(Card* target, Card* from){
	/* 구조체 데이터 복제 */
	target->month = from->month;
	target->index = from->index;
}

List* findCard(List** straw, List** target){
	/* straw에서 target과 일치하는 '월'을 가진 카드의 위치 리턴 */
	List* currentStraw;
	
	for(currentStraw = (*straw); currentStraw; currentStraw = currentStraw->next){
		if(currentStraw->data.month == (*target)->data.month){
			return currentStraw;
		}
	}

	currentStraw = (List*)malloc(sizeof(List));
	currentStraw->data.month = 0;
	return currentStraw;
}

List* findFee(List** straw){
	/* straw에서 피에 해당하는 카드를 찾는다 */
	List* currentStraw;
	
	for(currentStraw = (*straw); currentStraw; currentStraw = currentStraw->next){
		if(currentStraw->data.index == Fee){
			return currentStraw;
		}
	}

	/* 없을 경우 쌍피를 찾는다 */
	for(currentStraw = (*straw); currentStraw; currentStraw = currentStraw->next){
		if(currentStraw->data.index == Fee_d){
			return currentStraw;
		}
	}

	/* 없을 경우 껍데기 카드를 리턴한다 */
	currentStraw = (List*)malloc(sizeof(List));
	currentStraw->data.month = 0;
	return currentStraw;
}

void submitCard(List** straw, Player** target, int index, List** deck){
	/* 카드 제출 */
	int i;

	/* 최종 도달 리스트 설정 */
	List* dest = target[turn]->point_stack;
	bool sulsa = false;

	/* 제출할 카드 선택 */
	List* submit_target = target[turn]->card;
	
	for(i = 1; i < index; i++){
		submit_target = submit_target->next;
	}

	/* 흔들기 */
	List* startShake = findCard(&target[turn]->card, &submit_target);
	if(startShake->data.month != 0 && startShake->next != NULL && startShake->next->next != NULL
		&& startShake->data.month == startShake->next->data.month
		&& startShake->data.month == startShake->next->next->data.month){

		printf("흔드시겠습니까?(판돈두배) Yes or No ");
		printf("\n(1 or 2) : ");

		int command;
		while((command = getchar()) != EOF){
			if(command == 49){
				pay *= 2;
				break;
			}else if(command == 50){
				break;
			}
		}
	}

	/* 제출한 카드와 일치하는 월을 가진 카드를 바닥에 놓인 카드 중에서 찾음 */
	List* currentStraw = findCard(straw, &submit_target);

	if(currentStraw->data.month == 0){
		/* 카드가 없는 경우 */
		
		/* 뒤집은 카드로 먹을 것이 있는지 확인 */
		List* sameCardWithTurn = findCard(straw, deck);
		
		if(sameCardWithTurn->data.month == 0){
			/* 카드가 없을 경우 */

			if((*deck)->data.month == submit_target->data.month){
				/* 쪽 - 내놓은 카드와 뒤집은 카드가 일치 */

				/* 카드 이동 및 리스트 적용 */
				moveFromTo(deck, &dest);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(&submit_target, &dest);
				applyChangeList(submit_target, &target[turn]->card);
				applyChangeList(dest, &target[turn]->point_stack);
			}else{
				moveFromTo(&submit_target, straw);
				applyChangeList(submit_target, &target[turn]->card);

				moveFromTo(deck, straw);
			}
		}else if(sameCardWithTurn->next != NULL && sameCardWithTurn->next->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->next->data.month){
			/* 싼 것 먹기 - 뒤집은 카드가 바닥에 놓인 카드 3장 묶음과 일치할 경우 */
			
			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&submit_target, straw);
			applyChangeList(submit_target, &target[turn]->card);
	
			/* 피한장씩 걷기 */
			int i;
			for(i = 0; i < 3; i++){
				if(i != turn){
					List* fee = findFee(&target[i]->point_stack);
					moveFromTo(&fee, &dest);
					applyChangeList(fee, &target[i]->point_stack);
					applyChangeList(dest, &target[turn]->point_stack);
				}
			}


		}else if(sameCardWithTurn->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month){
			/* 카드 선택 - 뒤집은 카드가 바닥에 놓인 카드 2장 묶음과 일치할 경우 */
			
			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);

			printf("패 선택 : ");
			printf("%d", sameCardWithTurn->data.month); printCardIndexName(sameCardWithTurn->data.index); printf(" ");
			printf("%d", sameCardWithTurn->next->data.month); printCardIndexName(sameCardWithTurn->next->data.index);
			printf("\n(1 or 2) : ");

			int command;
			while((command = getchar()) != EOF){
				if(command == 50){
					sameCardWithTurn = sameCardWithTurn->next;
				}else if(command == 49){
					
				}

				if(command >= 49 && command <= 50){
					moveFromTo(&sameCardWithTurn, &dest);
					applyChangeList(sameCardWithTurn, straw);
					applyChangeList(dest, &target[turn]->point_stack);

					moveFromTo(&submit_target, straw);
					applyChangeList(submit_target, &target[turn]->card);

					break;
				}
			}

		}else{
			/* 뒤집은 카드가 바닥에 놓인 카드 1장과 일치할 경우 */

			moveFromTo(&submit_target, straw);
			applyChangeList(submit_target, &target[turn]->card);
			
			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

		}
	}else if(currentStraw->next != NULL && currentStraw->next->next != NULL
		&& currentStraw->data.month == currentStraw->next->data.month
		&& currentStraw->data.month == currentStraw->next->next->data.month){
		/* 3개 겹친 카드를 모두 가져올 경우 */

		moveFromTo(&submit_target, &dest);
		applyChangeList(submit_target, &target[turn]->card);
		applyChangeList(dest, &target[turn]->point_stack);

		moveFromTo(&currentStraw, &dest);
		applyChangeList(currentStraw, straw);
		applyChangeList(dest, &target[turn]->point_stack);

		moveFromTo(&currentStraw, &dest);
		applyChangeList(currentStraw, straw);
		applyChangeList(dest, &target[turn]->point_stack);

		moveFromTo(&currentStraw, &dest);
		applyChangeList(currentStraw, straw);
		applyChangeList(dest, &target[turn]->point_stack);

		/* 피한장씩 걷기 */
		int i;
		for(i = 0; i < 3; i++){
			if(i != turn){
				List* fee = findFee(&target[i]->point_stack);
				moveFromTo(&fee, &dest);
				applyChangeList(fee, &target[i]->point_stack);
				applyChangeList(dest, &target[turn]->point_stack);
			}
		}

		List* sameCardWithTurn = findCard(straw, deck);
		if(sameCardWithTurn->data.month == 0){
			moveFromTo(deck, straw);
		}else if(sameCardWithTurn->next != NULL && sameCardWithTurn->next->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->next->data.month){
			/* 싼 것 먹기 - 뒤집은 카드가 바닥에 놓인 카드 3장 묶음과 일치할 경우 */

			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			/* 피한장씩 걷기 */
			int i;
			for(i = 0; i < 3; i++){
				if(i != turn){
					List* fee = findFee(&target[i]->point_stack);
					moveFromTo(&fee, &dest);
					applyChangeList(fee, &target[i]->point_stack);
					applyChangeList(dest, &target[turn]->point_stack);
				}
			}

		}else if(sameCardWithTurn->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month){
			/* 카드 선택 - 뒤집은 카드가 바닥에 놓인 카드 2장 묶음과 일치할 경우 */

			printf("패 선택 : ");
			printf("%d", sameCardWithTurn->data.month); printCardIndexName(sameCardWithTurn->data.index); printf(" ");
			printf("%d", sameCardWithTurn->next->data.month); printCardIndexName(sameCardWithTurn->next->data.index);
			printf("\n(1 or 2) : ");

			int command;
			while((command = getchar()) != EOF){
				if(command == 50){
					sameCardWithTurn = sameCardWithTurn->next;
				}else if(command == 49){
					
				}

				if(command >= 49 && command <= 50){
					moveFromTo(&sameCardWithTurn, &dest);
					applyChangeList(sameCardWithTurn, straw);
					applyChangeList(dest, &target[turn]->point_stack);

					moveFromTo(deck, &dest);
					applyChangeList(dest, &target[turn]->point_stack);

					break;
				}
			}
		}else{
			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);
		}

	}else if(currentStraw->next != NULL
		&& currentStraw->data.month == currentStraw->next->data.month){
		/* 2개 겹친 카드 중 하나를 선택할 경우 */

		printf("패 선택 : ");
		printf("%d", currentStraw->data.month); printCardIndexName(currentStraw->data.index); printf(" ");
		printf("%d", currentStraw->next->data.month); printCardIndexName(currentStraw->next->data.index);
		printf("\n(1 or 2) : ");

		int command;
		while((command = getchar()) != EOF){
			if(command == 50){
				currentStraw = currentStraw->next;
			}else if(command == 49){
			}

			if(command >= 49 && command <= 50){
				moveFromTo(&currentStraw, &dest);
				applyChangeList(currentStraw, straw);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(&submit_target, &dest);
				applyChangeList(submit_target, &target[turn]->card);
				applyChangeList(dest, &target[turn]->point_stack);

				break;
			}
		}

		List* sameCardWithTurn = findCard(straw, deck);
		if(sameCardWithTurn->data.month == 0){

			moveFromTo(deck, straw);

		}else if(sameCardWithTurn->next != NULL && sameCardWithTurn->next->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->next->data.month){
			/* 싼 것 먹기 - 뒤집은 카드가 바닥에 놓인 카드 3장 묶음과 일치할 경우 */

			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			/* 피한장씩 걷기 */
			int i;
			for(i = 0; i < 3; i++){
				if(i != turn){
					List* fee = findFee(&target[i]->point_stack);
					moveFromTo(&fee, &dest);
					applyChangeList(fee, &target[i]->point_stack);
					applyChangeList(dest, &target[turn]->point_stack);
				}
			}

		}else if(sameCardWithTurn->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month){
			/* 카드 선택 - 뒤집은 카드가 바닥에 놓인 카드 2장 묶음과 일치할 경우 */

			printf("패 선택 : ");
			printf("%d", sameCardWithTurn->data.month); printCardIndexName(sameCardWithTurn->data.index); printf(" ");
			printf("%d", sameCardWithTurn->next->data.month); printCardIndexName(sameCardWithTurn->next->data.index);
			printf("\n(1 or 2) : ");

			int command;
			while((command = getchar()) != EOF){
				if(command == 50){
					sameCardWithTurn = sameCardWithTurn->next;
				}else if(command == 49){
					
				}

				if(command >= 49 && command <= 50){
					moveFromTo(&sameCardWithTurn, &dest);
					applyChangeList(sameCardWithTurn, straw);
					applyChangeList(dest, &target[turn]->point_stack);

					moveFromTo(deck, &dest);
					applyChangeList(dest, &target[turn]->point_stack);

					break;
				}
			}
		}else{
			if(currentStraw->data.month == sameCardWithTurn->data.month){
				/* 따닥 */

				moveFromTo(&currentStraw, &dest);
				applyChangeList(currentStraw, straw);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(deck, &dest);
				applyChangeList(dest, &target[turn]->point_stack);

				/* 피한장씩 걷기 */
				int i;
				for(i = 0; i < 3; i++){
					if(i != turn){
						List* fee = findFee(&target[i]->point_stack);
						moveFromTo(&fee, &dest);
						applyChangeList(fee, &target[i]->point_stack);
						applyChangeList(dest, &target[turn]->point_stack);
					}
				}

			}else{
				moveFromTo(&sameCardWithTurn, &dest);
				applyChangeList(sameCardWithTurn, straw);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(deck, &dest);
				applyChangeList(dest, &target[turn]->point_stack);
			}
		}
	}else if(currentStraw != NULL){
		/* 하나의 카드가 일치 할 경우 */

		List* sameCardWithTurn = findCard(straw, deck);
		if(sameCardWithTurn->data.month == 0){
			/* 뒤집은 카드와 일치하는 바닥에 놓인 카드가 없는 경우 */

			moveFromTo(&currentStraw, &dest);
			applyChangeList(currentStraw, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(deck, straw);
			moveFromTo(&submit_target, &dest);
			applyChangeList(dest, &target[turn]->point_stack);
			applyChangeList(submit_target, &target[turn]->card);

		}else if(sameCardWithTurn->next != NULL && sameCardWithTurn->next->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->next->data.month){
			/* 싼 것 먹기 - 뒤집은 카드가 바닥에 놓인 카드 3장 묶음과 일치할 경우 */

			moveFromTo(&submit_target, &dest);
			applyChangeList(submit_target, &target[turn]->card);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&currentStraw, &dest);
			applyChangeList(currentStraw, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(deck, &dest);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			moveFromTo(&sameCardWithTurn, &dest);
			applyChangeList(sameCardWithTurn, straw);
			applyChangeList(dest, &target[turn]->point_stack);

			/* 피한장씩 걷기 */
			int i;
			for(i = 0; i < 3; i++){
				if(i != turn){
					List* fee = findFee(&target[i]->point_stack);
					moveFromTo(&fee, &dest);
					applyChangeList(fee, &target[i]->point_stack);
					applyChangeList(dest, &target[turn]->point_stack);
				}
			}

		}else if(sameCardWithTurn->next != NULL
			&& sameCardWithTurn->data.month == sameCardWithTurn->next->data.month){
			/* 카드 선택 - 뒤집은 카드가 바닥에 놓인 카드 2장 묶음과 일치할 경우 */

			printf("패 선택 : ");
			printf("%d", sameCardWithTurn->data.month); printCardIndexName(sameCardWithTurn->data.index); printf(" ");
			printf("%d", sameCardWithTurn->next->data.month); printCardIndexName(sameCardWithTurn->next->data.index);
			printf("\n(1 or 2) : ");

			int command;
			while((command = getchar()) != EOF){
				if(command == 50){
					sameCardWithTurn = sameCardWithTurn->next;
				}else if(command == 49){
					
				}

				if(command >= 49 && command <= 50){
					moveFromTo(&sameCardWithTurn, &dest);
					applyChangeList(sameCardWithTurn, straw);
					applyChangeList(dest, &target[turn]->point_stack);

					moveFromTo(deck, &dest);
					applyChangeList(dest, &target[turn]->point_stack);

					moveFromTo(&submit_target, &dest);
					applyChangeList(submit_target, &target[turn]->card);
					applyChangeList(dest, &target[turn]->point_stack);

					moveFromTo(&currentStraw, &dest);
					applyChangeList(currentStraw, straw);
					applyChangeList(dest, &target[turn]->point_stack);
					
					break;
				}
			}
		}else{
			/* 뒤집은 카드와 바닥에 놓인 카드가 하나 일치할 경우 */

			if(currentStraw->data.month == sameCardWithTurn->data.month){
				/* 설사 */

				moveFromTo(&submit_target, straw);
				applyChangeList(submit_target, &target[turn]->card);

				moveFromTo(deck, straw);

				target[turn]->num_sulsa++;
				sulsa = true;
				if(cycle == 0){
					/* 첫뻑 */

					//3점에 해당하는 돈 뺏어옴
					int i;
					for(i = 0; i < 3; i++){
						if(i != turn){
							target[i]->money -= 3 * pay;
						}
					}
					target[turn]->money += 3 * pay * 2;

				}else if(cycle == 1 && target[turn]->cont_sulsa){
					/* 연뼉 */

					//5점에 해당하는 돈 뺏어옴
					int i;
					for(i = 0; i < 3; i++){
						if(i != turn){
							target[i]->money -= 5 * pay;
						}
					}
					target[turn]->money += 5 * pay * 2;
				}

				if(target[turn]->num_sulsa >= 5){
					/* 게임 끝 */

				}
			}else{
				/* 뒤집은 카드와 일치하는 카드 하나 회수 */

				moveFromTo(&submit_target, &dest);
				applyChangeList(submit_target, &target[turn]->card);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(&currentStraw, &dest);
				applyChangeList(currentStraw, straw);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(deck, &dest);
				applyChangeList(dest, &target[turn]->point_stack);

				moveFromTo(&sameCardWithTurn, &dest);
				applyChangeList(sameCardWithTurn, straw);
				applyChangeList(dest, &target[turn]->point_stack);
			}
		}
	}

	/* 게임 중 설사가 일어나면 플레이어 설사 True */
	if(sulsa == true){
		target[turn]->cont_sulsa = true;
	}else{
		target[turn]->cont_sulsa = false;
	}

	if(*straw == NULL){
		/* 판 쓸이 */

		/* 피한장씩 걷기 */
		int i;
		for(i = 0; i < 3; i++){
			if(i != turn){
				List* fee = findFee(&target[i]->point_stack);
				moveFromTo(&fee, &dest);
				applyChangeList(fee, &target[i]->point_stack);
				applyChangeList(dest, &target[turn]->point_stack);
			}
		}
	}
}

void applyChangeList(void* copy, List** real){
	/* moveFromTo 함수를 통해 변한 리스트 적용 */
	List* ccopy = (List*)copy;

	if(ccopy == NULL){
		(*real) = NULL;
		return;
	}

	while(ccopy->prev != NULL){
		ccopy = ccopy->prev;
	}
	
	(*real) = ccopy;
}

void moveFromTo(List** from, List** to){
	/* From에서 To에 리스트 노드를 한개 옮김 */

	List* tmp = NULL;
	if((*from)->next != NULL){
		tmp = (*from)->next;
	}else if((*from)->prev != NULL){
		tmp = (*from)->prev;
	}

	if((*from) == NULL){
		return;
	}

	if((*from)->prev == NULL && (*from)->next == NULL){
		if((*to) == NULL){
			(*to) = (*from);
			(*from) = NULL;
		}else{
			(*from)->next = (*to);
			(*to)->prev = (*from);
			(*to) = (*to)->prev;

			(*from) = NULL;
		}
		return;
	}else if((*from)->prev == NULL){
		if((*to) == NULL){
			(*from)->next = NULL;
			(*to) = (*from);
		}else{
			(*from)->prev = NULL;
			(*from)->next = (*to);
			(*to)->prev = (*from);
			(*to) = (*to)->prev;
			(*to)->prev = NULL;
		}
		tmp->prev = NULL;
	}else if((*from)->next == NULL){
		if((*to) == NULL){
			(*from)->next = NULL;
			(*to) = (*from);
		}else{
			(*from)->prev = NULL;
			(*from)->next = (*to);
			(*to)->prev = (*from);
			(*to) = (*to)->prev;
			(*from) = tmp;
			(*from)->next = NULL;
		}
	}else if(((*from)->prev != NULL) && ((*from)->next != NULL)){
		(*from)->prev->next = (*from)->next;
		(*from)->next->prev = (*from)->prev;
		
		if((*to) == NULL){
			(*from)->next = NULL;
			(*to) = (*from);
		}else{
			(*from)->next = (*to);
			(*to)->prev = (*from);
			(*to) = (*to)->prev;
		}
	}

	(*to)->prev = NULL;
	if(tmp == NULL){
		(*from) = NULL;
	}else{
		(*from) = tmp;
	}
}

List* setDeck(Card* card){
	int i;

	/* 카드 배열을 섞음 */
	for(i = 0; i < 48; i++){
		int randcnt = rand() % 48;
		Card tmp = card[i];
		card[i] = card[randcnt];
		card[randcnt] = tmp;
	}

	/* 리스트 노드에 카드 배열의 값을 대입 */
	List* head = NULL;
	for(i = 0; i < 48; i++){
		head = addNode(head, card[i]);
	}

	return head;
}

List* addNode(List* head, Card data){
	/* 노드 할당 & 추가 */

	List* node = (List*)malloc(sizeof(List));
	_cardDataMapping(&node->data, &data);
	node->prev = NULL;
	node->next = NULL;

	if(head == NULL){
		return node;
	}
	node->next = head;
	head->prev = node;
	return node;
}

void sortList(List** list){
	/* 리스트 월 순 정렬 */

	List* i = NULL;
	List* j = NULL;

	for(i=(*list); i; i = i->next){
		for(j=i->next; j; j = j->next){
			if(i->data.month > j->data.month){
				switchList(&i, &j);
			}
		}
	}
}

void switchList(List** tar1, List** tar2){
	/* 리스트 값 위치 변경 */

	Card tmp = (*tar1)->data;
	_cardDataMapping(&(*tar1)->data, &(*tar2)->data);
	_cardDataMapping(&(*tar2)->data, &tmp);
}

/* Calculate point */

void cal_point(Player ** player){
	/* 점수 계산 */

	int i;
	for(i=0;i<3;i++)
	player[i]->point=0;

	for(i=0;i<3;i++)
	{
		//피 점수 계산
		if(player[i]->Fee_count+player[i]->Fee_d_count>=10 && player[i]->Fee_count+player[i]->Fee_d_count<20){
			player[i]->point+=((player[i]->Fee_count+player[i]->Fee_d_count)-9)+1;
		}else if(player[i]->Fee_count+player[i]->Fee_d_count>=20&&player[i]->Fee_count+player[i]->Fee_d_count<30){
			player[i]->point+=((player[i]->Fee_count+player[i]->Fee_d_count)-9)+11;
		}
		//띠 점수 계산
		if(player[i]->Band_r_count==3)
		{
			player[i]->point+=3;
		}
		if(player[i]->Band_b_count==3)
		{
			player[i]->point+=3;
		}
		if(player[i]->Band_c_count==3)
		{
			player[i]->point+=3;
		}
		if(player[i]->Band_b_count+player[i]->Band_c_count+player[i]->Band_r_count+player[i]->Band_x_count>=5)
		{
			player[i]->point+=((player[i]->Band_b_count+player[i]->Band_c_count+player[i]->Band_r_count+player[i]->Band_x_count)%5)+1;
		}
		//고도리
		if(player[i]->Animal_b_count==3)
		{
			player[i]->point+=5;
		}
		//열끗
		if(player[i]->Animal_count+player[i]->Animal_b_count>=5)
		{
			player[i]->point+=((player[i]->Animal_count+player[i]->Animal_b_count)-5)+1;

		}
		//광 
		if(player[i]->Kwang_count+player[i]->Kwang_x_count==3)
		{
			if(player[i]->Kwang_x_count==1)
			{
				player[i]->point+=2;
			}
			else
				player[i]->point+=3;
		}
		else if(player[i]->Kwang_count+player[i]->Kwang_x_count==4)
		{
			player[i]->point+=4;
		}
		else if(player[i]->Kwang_count+player[i]->Kwang_x_count==5)
		{
			player[i]->point+=15;
		}
	
		   if(player[i]->go==1)
		   {
		   player[i]->point+=1;
		   }
		   else if(player[i]->go==2)
		   {
		   player[i]->point+=2;
		   }
		   else if(player[i]->go==3)
		   {
		   player[i]->point=(player[i]->point+2)*2;
		   }
		   else if(player[i]->go==4)
		   {
		   player[i]->point=(player[i]->point+2)*4;
		   }
		//멍박
		if((player[i]->Animal_b_count+player[i]->Animal_count)>=7)
		{
		player[i]->point=player[i]->point*2;
		}
		//흔들기
		if(player[i]->shake==1)
			player[i]->point *=2;
		if(player[i]->shake==2)
			player[i]->shake *=4;
	}
}

void cal_card_count(Player ** player){
	/* 카드 조합 검사 */

	int result[3];
	int i,j;

	//광 개수 저장/
	get_count(player,1,result);
	for(i=0;i<3;i++)
	{
		player[i]->Kwang_count=result[i];
	}
	//홍단 개수 저장/
	get_count(player,2,result);
	for(i=0;i<3;i++)
	{
		player[i]->Band_r_count=result[i];
	}
	//청단 개수 저장/
	get_count(player,3,result);
	for(i=0;i<3;i++)
	{
		player[i]->Band_b_count=result[i];
	}
	//초단 개수 저장/
	get_count(player,4,result);
	for(i=0;i<3;i++)
	{
		player[i]->Band_c_count=result[i];
	}
	//12월 띠 개수 저장/
	get_count(player,5,result);
	for(i=0;i<3;i++)
	{
		player[i]->Band_x_count=result[i];
	}
	//열끗
	get_count(player,6,result);
	for(i=0;i<3;i++)
	{
		player[i]->Animal_count=result[i];
	}
	//고도리
	get_count(player,7,result);
	for(i=0;i<3;i++)
	{
		player[i]->Animal_b_count=result[i];
	}
	//쌍피
	get_count(player,8,result);
	for(i=0;i<3;i++)
	{
		player[i]->Fee_d_count=result[i] * 2;
	}
	//피
	get_count(player,0,result);
	for(i=0;i<3;i++)
	{
		player[i]->Fee_count=result[i];
	}
	//비광
	get_count(player,9,result);
	for(i=0;i<3;i++)
	{
		player[i]->Kwang_x_count=result[i];
	}
}

void get_count(Player ** player, int n, int * result){
	/* 인덱스 갯수 파악 */

	int i,j;
	int count=0;

	for(i=0;i<3;i++){
		while(1){
			if(player[i]->point_stack == NULL){
				break;
			}else{
				if(player[i]->point_stack->data.index==n){
					count++;
				}
				player[i]->point_stack=player[i]->point_stack->next;
				if(player[i]->point_stack != NULL && player[i]->point_stack->next==NULL){
					if(player[i]->point_stack->data.index==n)
						count++;
					while(1){
						player[i]->point_stack=player[i]->point_stack->prev;
						if(player[i]->point_stack->prev==NULL)
							break;
					}
					break;
				}
			}
		}
		result[i]=count;
		count=0;
	}
}

void cal_money(Player ** player){
	/* 점수에 따른 돈계산 */

	int payEachPlayer[3];
	int goFreq = 0;
	int goPlayerIndex = 0;
	int i;
	for(i = 0; i < 3; i++){
		int doubleCnt = 0;
		payEachPlayer[i] = 0;
		if(i != turn){
			if((player[i]->Fee_count + player[i]->Fee_d_count) <= 5
				&& (player[turn]->Fee_count + player[turn]->Fee_d_count) >= 10){
				doubleCnt++;
			}

			if((player[i]->Kwang_count + player[i]->Kwang_x_count) <= 0
				&& (player[turn]->Kwang_count + player[turn]->Kwang_x_count) >= 3){
				doubleCnt++;
			}

			if((player[i])->go >= 1){
				goFreq++;
				goPlayerIndex = i;
			}

			payEachPlayer[i] = pay * player[turn]->point * pow((double)2, doubleCnt);
		}
	}

	if(goFreq == 1){
		payEachPlayer[goPlayerIndex] = payEachPlayer[0] + payEachPlayer[1] + payEachPlayer[2];
		for(i = 0; i < 3; i++){
			if(i != goPlayerIndex){
				payEachPlayer[i] = 0;
			}
		}
	}
	
	for(i = 0; i < 3; i++){
		player[i]->money -= payEachPlayer[i];
		player[turn]->money += payEachPlayer[i];
	}

	return;
}


/* File I/O */
void saveFile(List** deck, List** ground, Player** player){
	/* 게임 저장 */

	FILE* fp = NULL;
	fp = fopen("save.drg", "wb");

	List* arm;
	int i;
	for(i = 0; i < 3; i++){
		int cardN = getListNum(player[i]->card);
		fwrite(&cardN, sizeof(int), 1, fp);

		for(arm = player[i]->card; arm; arm = arm->next){
			fwrite(&arm->data, sizeof(Card), 1, fp);
		}

		cardN = getListNum(player[i]->point_stack);
		fwrite(&cardN, sizeof(int), 1, fp);

		for(arm = player[i]->point_stack; arm; arm = arm->next){
			fwrite(&arm->data, sizeof(Card), 1, fp);
		}
	}

	int deckN = getListNum(*deck);
	int groundN = getListNum(*ground);

	fwrite(&deckN, sizeof(int), 1, fp);
	for(arm = (*deck); arm; arm = arm->next){
		fwrite(&arm->data, sizeof(Card), 1, fp);
	}

	fwrite(&groundN, sizeof(int), 1, fp);
	for(arm = (*ground); arm; arm = arm->next){
		fwrite(&arm->data, sizeof(Card), 1, fp);
	}

	for(i = 0; i < 3; i++){
		fwrite(&player[i]->go, sizeof(int), 1, fp);
		fwrite(&player[i]->shake, sizeof(int), 1, fp);
		fwrite(&player[i]->money, sizeof(int), 1, fp);
		fwrite(&player[i]->num_sulsa, sizeof(int), 1, fp);
		fwrite(&player[i]->cont_sulsa, sizeof(bool), 1, fp);
	}

	fwrite(&turn, sizeof(int), 1, fp);
	fwrite(&start_turn, sizeof(int), 1, fp);
	fwrite(&cycle, sizeof(int), 1, fp);
	fclose(fp);
}

void loadFile(List** deck, List** ground, Player** player){
	/* 게임 로드 */

	int i, j;
	FILE* fp = NULL;
	if((fp = fopen("save.drg", "rb")) == NULL){
		printf("No save data");
		return;
	}

	clear(deck);
	clear(ground);
	for(i = 0; i < 3; i++){
		clear(&(*player[i]).card);
		clear(&(*player[i]).point_stack);
		player[i]->point = 0;
		player[i]->num_sulsa = 0;
		player[i]->cont_sulsa = false;
		player[i]->go = 0;
		player[i]->shake = 0;
	}

	for(i = 0; i < 3; i++){
		Card data;
		int cnt;
		fread(&cnt, sizeof(int), 1, fp);
		for(j = 0; j < cnt; j++){
			fread(&data, sizeof(Card), 1, fp);
			player[i]->card = addNode(player[i]->card, data);
		}
		fread(&cnt, sizeof(int), 1, fp);
		for(j = 0; j < cnt; j++){
			fread(&data, sizeof(Card), 1, fp);
			player[i]->point_stack = addNode(player[i]->point_stack, data);
		}
	}

	int deckN;
	int groundN;

	fread(&deckN, sizeof(int), 1, fp);
	for(i = 0; i < deckN; i++){
		Card data;
		fread(&data, sizeof(Card), 1, fp);
		(*deck) = addNode((*deck), data);
	}

	fread(&groundN, sizeof(int), 1, fp);
	for(i = 0; i < groundN; i++){
		Card data;
		fread(&data, sizeof(Card), 1, fp);
		(*ground) = addNode((*ground), data);
	}

	for(i = 0; i < 3; i++){
		int tmpCnt;

		fread(&tmpCnt, sizeof(int), 1, fp);
		player[i]->go = tmpCnt;
		printf("%d ", tmpCnt);

		fread(&tmpCnt, sizeof(int), 1, fp);
		player[i]->shake = tmpCnt;
		printf("%d ", tmpCnt);

		fread(&tmpCnt, sizeof(int), 1, fp);
		player[i]->money = tmpCnt;
		printf("%d ", tmpCnt);

		fread(&tmpCnt, sizeof(int), 1, fp);
		player[i]->num_sulsa = tmpCnt;
		printf("%d ", tmpCnt);

		fread(&tmpCnt, sizeof(bool), 1, fp);
		player[i]->cont_sulsa = tmpCnt;
		printf("%d\n", tmpCnt);
	}
	fread(&i, sizeof(bool), 1, fp);
	turn = i - 1;
	fread(&i, sizeof(bool), 1, fp);
	start_turn = i;
	fread(&i, sizeof(bool), 1, fp);
	cycle = i;

	if(turn == start_turn){
		cycle--;
	}

	fclose(fp);

	draw(player, (*ground));
}
