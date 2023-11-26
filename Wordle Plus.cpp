#define _CRT_SECURE_NO_WARNINGS
//Used for exit constants mostly
#include <stdlib.h>
//printf and puts
#include <stdio.h>
//string comparison functions
#include <string.h>
//Used for the function 'toLower' 
#include <ctype.h> 
//Used to present the outcomes with different colors && play background music
#include <windows.h>
//Used for getch() in double-player mode
#include <conio.h>
//Used to seed the rng for random games && calculate the game time
#include <time.h>
//Used to play background music
#include<mmsystem.h>
#pragma  comment (lib,"Winmm.lib") 


//Length of a answer.
//Note: You cannot change this without changing the answer list too.
//You also must adapt the scanf calls for the new length
#define answer_LENGTH 5
//Number of tries allowed to find the answer
#define MAX_TRIES 6
//Number of characters in the alphabet + 1
#define ALPHA_SIZE 27
//If set, the answer and a few stats are printed before the game starts
//#define DEBUG
//Number of answers in the all potential answer list + 1
#define ALL_SIZE 12948
//Number of answers in the solution list + 1
#define SOLUTION_SIZE 2316
//Note: CRLF is also used for telnet.
//If you want to make it available in a BBS you may want to force a CRLF
#ifdef WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif


//Files for lists that contain all answers and solutions
FILE * fpA,  * fpS;
//Number of answers in the solution list
long answerNum = 0;
//The numbers of attempts 
int AttemptNum = 0;
//Selected answer from solution list
char answer[answer_LENGTH + 1] = {0};
//Possible characters (used to show unused characters)
//The size specifier is necessary or its value will be readonly
char alpha[ALPHA_SIZE] ={"abcdefghijklmnopqrstuvwxyz"};
char ALPHA[ALPHA_SIZE] ={"abcdefghijklmnopqrstuvwxyz"};
//Memory cache:
//0-25 File position in the complete list with answers that start with the given letter a-z
//26: Number of answers in the solution list
long MemCache[ALPHA_SIZE];


//Checks the entered answer against the solution
int answerChecker(const char* guess);
//Removes characters in the supplied argument from the alphabet
int toLower(char* str);
//Checks if the entered string is a potentially valid answer
int allLetter(const char* answer);
//Checks if the supplied answer is in the list
int inList(const char * answer);
//Runs the main game loop.Removes characters in the supplied argument from the alphabet
//gameloop for single mode
void wordleLoop_single(void);
//gameloop for double mode
int wordleLoop_double(void);
//Runs the menu
int menu(void);
//Shows the help text
void help(void);
//Get a answer from ALL.TXT randomly
void random(void);
//Choose the game mode(single player or double player)
int modeChooser(void);
//Pick the given answer
int pickAnswer(char * answer, int index);
//Play background music
void playMusic(void);
//Input the answer in double-player mode with a '*' presented on the screen
void Answer(char *word, int Len);


int main(){
	fpA = fopen("D:\\ALL.TXT", "r");
	fpS = fopen("D:\\SOLUTION.TXT", "r");
	//Play background music
	playMusic();
	//Choose game mode£¨singleplayer or doubleplayer)
	int l = modeChooser();
	if(l < 0){
		return EXIT_SUCCESS;
	}
	else if(l > 0){
	    fclose(fpA);
		fclose(fpS);	
		return 0;
	}
	else{
		int z;
		while(l == 0){
			z = modeChooser();
			if(z < 0){
				return EXIT_SUCCESS;
			}
			else if(z > 0){
	    		fclose(fpA);
				fclose(fpS);	
				return 0;
			}
			else{
				continue;
			}
		}	
	}
}


int modeChooser(){
	int gamenum;
    gamenum = menu();
    //single mode
    if (gamenum > 0) {
		pickAnswer(answer, gamenum);
		#ifdef DEBUG
		//printf("answer: %s" EOL, answer);
		#endif
		//Randomly get a answer
		random();
		//puts(answer);
		//Calculate game time
		clock_t start, end;
		start = clock();
		wordleLoop_single();
		end = clock();
		float time = (end - start) / CLOCKS_PER_SEC;
		printf("time = %2f seconds\n",time);
		return 1;
	} 
	//double mode
	else if(gamenum == 0){
		int round;
		int outcome_1,outcome_2;
		int AttemptNum_1,AttemptNum_2;
		float time_1,time_2;
		for(round = 1;round < 3;round ++){
			printf("Player %d , please enter your answer.\n",round);
			Answer(answer, answer_LENGTH + 1);
			toLower(answer);
			if(allLetter(answer) && inList(answer)){
				printf("Player %d , now start your game!\n" ,3 - round);
				//Calculate game time
				clock_t start, end;
				start = clock();
				if(round == 1){
					outcome_1 = wordleLoop_double();
					AttemptNum_1 = AttemptNum;
				}
				else{
					outcome_2 = wordleLoop_double();
					AttemptNum_2 = AttemptNum;
				}
				end = clock();
				float time = (end - start) / CLOCKS_PER_SEC;
				if(round == 1){
					time_1 = time;
				}
				else{
					time_2 = time;
				}
				printf("time%d = %2f seconds\n\n", round,time);
		    }
			else{
				printf("Invalid answer!!!\nRestart the game.\n" EOL);
				return 0;
			}
		}	
		if(outcome_1 == 0 && outcome_2 == 1){
			printf("Player 1 win the game!");
		}
		else if(outcome_1 == 1 && outcome_2 == 0){
			printf("Player 2 win the game!");
		}
		else if(outcome_1 == 0 && outcome_2 == 0){
			printf("The game is tied!");
		} 
		else{
			if(AttemptNum_1 < AttemptNum_2){
				printf("Player 2 win the game for fewer attempts!");
			}
			else if(AttemptNum_1 > AttemptNum_2){
				printf("Player 1 win the game for fewer attempts!");
			}
			else{
				if(time_1 == time_2){
					printf("The game is tied!");
				}
				else if(time_1 <time_2){
					printf("Player 2 win the game for using less time!");
				}
				else{
					printf("Player 1 win the game for using less time!");
				}
			}
		return 1;
		}
	}
	//Error
	else {
		return -1;
	}
}


void Answer(char *word, int Len){
    char ch;
    int i = 0;
    while(i<Len)
    {
        ch = getch();
        //Press the 'Enter' button 
        if(ch == '\r'){  
            printf("\n");
            break;
        }
        //Press the 'Backspace' button
        if(ch=='\b' && i>0){  
            i--;
            printf("\b \b");
        }
        //Input the string with '*' presented on the screen
        else if(isprint(ch)){  
            answer[i] = ch;
            printf("*");
            i++;
        }
    }
    answer[i] = 0;
}


void playMusic(){
	//!!!precondition£ºin Dev-C++£¬Tools£­Compiler Options£­General£­linker-caller£ºadd ' -lwinmm'(a space is neended before -lwinmm) )
	//the music must be'.wav' file
	//enter certain	file path
	PlaySound(TEXT("E:\\music01.wav"),  NULL, SND_FILENAME | SND_ASYNC| SND_LOOP);
	printf("now playing : Go Home\n" EOL) ;
	}


int menu(){
	printf("Please read the game description in 'introduction' carefully before your first game. \n"EOL);
	char Buffer[21];
 	int scan = 0;
	puts(
  	"Menu:" EOL
  	"\tPlease choose:" EOL
  	"\tsingle:  single player mode" EOL
  	"\tdouble:  double player mode" EOL
  	"\thelp:  information about the game" EOL
  	"\texit:  end game"EOL);
 	while (true) {
  		puts("Please enter your choice:");
 			while ((scan = scanf("%20s", Buffer)) != 1) {
  				if (scan == EOF) {
   					return -1;
   				}
  			}
  			if (strcmp(Buffer, "exit") == 0) {
   				return -1;
  			}
  			else if (strcmp(Buffer, "help") == 0) {
   				help();
  			}
  			else if (strcmp(Buffer, "single") == 0) {
   				return 1;
  			}
  			else if (strcmp(Buffer, "double") == 0) {
   				return 0;
  			}
  			else {
  				 puts("Invalid input");
  			}
 	}
}


void random(){
    if ((fpA != NULL) && (fpS != NULL)){
		char s[6] = { 1,1,1,1,1 };
        int i,a = 0;
        //Measure the number of answers in the text
        for (i = 1; feof(fpS) == 0; i++) {
            while (a <= 5) {
    			s[a] = fgetc(fpS);
    			a++;
   			}
  			 a = 0;
 		 }
  		//Initialize the pointer position
  		fseek(fpS, 0, SEEK_SET);
  		//Initialize the random number generator
  		srand(time(NULL));   
  		//Random Seed£¬get random numbers
  		//Randomly get a answer
  		int e = rand() % i;
  		for (i = 1; i < e; i++) {
   			while (a <= 5) {
    			s[a] = fgetc(fpS);
   				 a++;
  			 }
  			a = 0;
  		}
  		//put the localized character into 'answer'
  		for (int i = 0; i <= answer_LENGTH; i++) {
  			 answer[i] = fgetc(fpS);
  		}
  		//Pointer position initialization
  		fseek(fpS, 0, SEEK_SET);
  		printf("\n");
  		#if debug
 		//puts(answer);
 		#endif
	}
}


int answerChecker(const char *guess){
	int i = 0;
	char copy[answer_LENGTH + 1];
	char outcome[answer_LENGTH + 1] ="";
    //set up a copy of 'answer' to avoid wrong reports for double letters, for example "l" in "balls"
	for (int t = 0; t < answer_LENGTH + 1; t++) {
		copy[t] = answer[t];
	}
	for (int t = 0; guess[t] != '\0'; t++) {
		//character found and position right
		if (guess[t] == copy[t]) {
			//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
			outcome[t] = guess[t];
			printf("\033[48;5;2m%c\033[m", outcome[t]);
			copy[t] = '_';
		}
		else {
			int flag = 0;
			for (int h = 0; h < answer_LENGTH + 1; h++) {
				//character found but position wrong
				if (guess[t] == copy[h]) {
					//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
					outcome[t] = guess[t];
					printf("\033[48;5;3m%c\033[m", outcome[t]);
					flag = 1;
					break;
				}
				else {
					continue;
				}
			}
			//character not found
			if(flag == 0) {
				//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
				outcome[t] = guess[t];
				printf("\033[48;5;1m%c\033[m", outcome[t]);
			}
		}
	}
	return 1;
}


int toLower(char* str){
	int i = 0;
	int len;
	len = strlen(str);
	for (i = 0; i < len; i++) {
		str[i] = tolower(str[i]);
	}
	return 0;
}


int allLetter(const char* answer){
    int sum = 0;
    if(strlen(answer) == answer_LENGTH){
    	int i;
        for (i=0;i<answer_LENGTH;i++){
            if(answer[i]>='a' && answer[i]<='z'){
                sum += 1;
            }
        }
        if (sum == answer_LENGTH){
            return 1;
        }
    }
    return 0;
}


int inList(const char * answer){
    //Buffer also contains '\n'
    char Buffer[answer_LENGTH + 4];
    if (answer != NULL && strlen(answer) == answer_LENGTH && allLetter(answer)) {
        //search from the answers with the same start
        fseek(fpA, MemCache[answer[0]-97], SEEK_SET); //ASCII of 'a' :97
        int stop = answer[0] + 1,tot = 0;
        char answers[ALL_SIZE][answer_LENGTH + 1];
        while (fgets(Buffer, answer_LENGTH + 4, fpA) && Buffer[0] < stop){
            Buffer[answer_LENGTH] = '\0';
             //get a valid answer to compare
            strcpy(answers[tot++], Buffer);
        }
        int low = 0, high = tot - 1,mid = 0,cmp = 0;
        while (low <= high){
            mid = (low + high) / 2;
            cmp = strcmp(answer, answers[mid]);
            if (cmp == 0){
                return 1; 
				//the supplied answer is in the list
            }else if (cmp < 0){
                high = mid - 1;
            }else{
                low = mid + 1;
            }
        }
    }
    return 0;
}


int pickAnswer(char * answer, int index){
	int i = 0;
	fseek(fpS, 0, SEEK_SET);
	while (i <= index && fgets(answer, answer_LENGTH + 1, fpS) != NULL) {
		if (strlen(answer) == answer_LENGTH) {
			++i;
		}
	}
	return index;
}


void wordleLoop_single(){
    char guess[answer_LENGTH + 10] = {0};
    int AttemptNum = 0;
    int in = 0;
    answer[5]='\0';
    puts("answer\tunused alphabet");
    while (AttemptNum < MAX_TRIES){
        if (!strcmp(guess, answer)){
            break;
    	}
        printf("\nGuess %i: ", AttemptNum + 1);
        if ((in = scanf("%s", guess)) == 1 && strlen(guess) == answer_LENGTH ){
            toLower(guess);
            if (strcmp(guess, answer)){
                if (allLetter(guess) && inList(guess)){
                    AttemptNum++;
                    if (answerChecker(guess)){
                        int i;
                        //when the character found in alpha , replace it with '_'
                        for(i = 0;i < answer_LENGTH + 1;i++){
                        	int t;
                            for(t = 0;t < ALPHA_SIZE - 1;t++){
                                if(alpha[t] == guess[i]){
                                    alpha[t] = '_';
                                }
                            }
                        }
                        printf("\t");
                        int q;
                        for(q = 0;q < ALPHA_SIZE + 1;q++){
                        	if(alpha[q] == '_'){
                        		printf("\033[48;5;1m%c\033[m",ALPHA[q]);
							}
							else{
								printf("%c",alpha[q]);
							}
						}
						#if debug
                        //printf("\t%s\n", alpha);
                        #endif
                    }
                } else {
                    puts("answer is not in list!");
					fflush(stdin);
                }
            }
        } 
		else {
            if (in == EOF) {
                exit(EXIT_FAILURE);
            }
            printf("\nInvalid answer. Must be %i characters\n", answer_LENGTH);
        }
    }
    if (!strcmp(guess, answer)){
        printf("\nYou win!\n");
    } 
	else {
        printf("\nYou lose. The answer was %s\n",answer);
    }
}


int wordleLoop_double(){
    char guess[answer_LENGTH + 10] = {0};
    AttemptNum = 0;
    int in = 0;
    answer[5]='\0';
    puts("answer\tunused alphabet");
    while (AttemptNum < MAX_TRIES){
        if (!strcmp(guess, answer)){
            break;
    	}
        printf("\nGuess %i: ", AttemptNum + 1);
        if ((in = scanf("%s", guess)) == 1 && strlen(guess) == answer_LENGTH ){
            toLower(guess);
            if (strcmp(guess, answer)){
                if (allLetter(guess) && inList(guess)){
                    AttemptNum++;
                    if (answerChecker(guess)){
                        int i;
                        //when the character found in alpha , replace it with '_'
                        for(i = 0;i < answer_LENGTH + 1;i++){
                        	int t;
                            for(t = 0;t < ALPHA_SIZE - 1;t++){
                                if(alpha[t] == guess[i]){
                                    alpha[t] = '_';
                                }
                            }
                        }
                        printf("\t"); 
                        int q;
                        for(q = 0;q < ALPHA_SIZE + 1;q++){
                        	if(alpha[q] == '_'){
                        		printf("\033[48;5;1m%c\033[m",ALPHA[q]);
							}
							else{
								printf("%c",alpha[q]);
							}
						}
                    }
                } else {
                    puts("answer is not in list!");
					fflush(stdin);
                }
            }
        } 
		else {
            if (in == EOF) {
                exit(EXIT_FAILURE);
            }
            printf("\nInvalid answer. Must be %i characters\n", answer_LENGTH);
        }
    }
    if (!strcmp(guess, answer)){
        printf("\nSuccess\n");
        return 1;
    } 
	else {
        printf("\nFailure. The answer was %s\n",answer);
        return 0;
    }
}


void help(){
	printf(
			"------------------------------------------------------------------------------------------------------------------------\n"
	        "A brief introduction to 'wordle'\n"EOL
	        "In 2022 ,the top search keyword in the global hot search content is 'Wordle'"
	        "But our 'Wordle' is different (with more complete functions) to some extent."
	        "There are two modes to be chosen:\n"
			"1.Single player mode : \n\tThe computer picks a answer at random as the 'answer'\n"
			"\tGuess the 5 letters of the answer within 6 tries,and then you win.\n"
			"2.Double player mode £º\n\tOne player( answerle-maker ) enter a answer as the 'answer', the other( answerle-guesser ) guess the answer\n"
			"\tThen the two players switch roles.\n"
			"\tIf one of them guess out the answer successfully while the other fail,the one who succeed is the winner.\n "
			"\tIf they both guess out the answer ,then check their number of attempts ,the player with less attempts is the winner.\n"
			"\tIf they both guess out the answer with the same number of attempts ,then check their time ,the player using less time is the winner."
			"\tOtherwise the game is tied\n"
			"You can choose one mode.\n" EOL
			);
	puts(
	    	"After every guess,each character will be shown in a certain color.\n" 
		    "They will be presented like this:\n" 
		    "  \033[48;5;1m  red  \033[m    means Character not found at all\n" 
		    "  \033[48;5;2m green \033[m  means Character found and position correct\n"
		    "  \033[48;5;3m yellow\033[m means Character found but position wrong\n" 
		    "Unused letters of the alphabet are shown next to the hint\n" 
		    "The game prefers valid positions over invalid positions,\n" 
		    "And it handles double letters properly.\n" 
		    "Guessing \"RATES\" when the answer is \"TESTS\" shows \"\033[48;5;1mRA\033[m\033[48;5;3mTE\033[m\033[48;5;2mS\033[m\"\n"
			"------------------------------------------------------------------------------------------------------------------------\n"
			);
}
