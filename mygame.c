#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>

#define RESET  "\033[0m"
#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define CYAN   "\033[36m"

// ------- RAW MODE -------
struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// --------- KBHIT ----------
int kbhit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

// -------- CLEAR SCREEN (Mac safe) --------
void clearScreen() {
    system("clear");
}

int main() {
    srand(time(NULL));
    enableRawMode();

    char again;

    do {
        int x = 1; // Player: lane 0,1,2
        int step = 0;
        int obstaclePos = rand() % 3;
        int score = 0;
        int lives = 3;
        int gameOver = 0;
        int level = 1;
        int speed = 130000; // starting speed

        while (!gameOver) {

            // INPUT ------
            if (kbhit()) {
                char c = getchar();
                if ((c == 'a' || c == 'A') && x > 0) x--;
                if ((c == 'd' || c == 'D') && x < 2) x++;
                if (c == 'q' || c == 'Q') {
                    gameOver = 1;
                    lives = 0;
                }
            }

            // DRAW GAME ----
            clearScreen();

            printf(YELLOW "Controls: A=Left | D=Right | Q=Quit\n" RESET);
            printf(YELLOW "Score: %d   Lives: %d   Level: %d\n" RESET, score, lives, level);
            printf(CYAN  "|--- --- ---|\n" RESET);

            for (int i = 0; i < 10; i++) {
                if (i == step) {
                    if (obstaclePos == 0) printf("| " RED "X" RESET "        |\n");
                    if (obstaclePos == 1) printf("|    " RED "X" RESET "     |\n");
                    if (obstaclePos == 2) printf("|        " RED "X" RESET " |\n");
                } else {
                    printf("|          |\n");
                }
            }

            if (x == 0) printf("| " GREEN "O" RESET "        |\n");
            else if (x == 1) printf("|    " GREEN "O" RESET "     |\n");
            else printf("|        " GREEN "O" RESET " |\n");

            fflush(stdout);

            // COLLISION CHECK ------
            if (step == 10 && x == obstaclePos) {
                lives--;
                if (lives <= 0) gameOver = 1;
                else {
                    step = 0;
                    obstaclePos = rand() % 3;
                }
            }

            usleep(speed);
            step++;

            // If passed successfully
            if (step > 10) {
                step = 0;
                obstaclePos = rand() % 3;
                score++;

                // Auto difficulty
                if (speed > 40000) {
                    speed -= 3000;
                }
                level = (130000 - speed) / 10000 + 1;
            }
        }

        // GAME OVER SCREEN ----
        clearScreen();
        printf(RED "\nGAME OVER!" RESET " Final Score: " YELLOW "%d\n" RESET, score);
        if (lives <= 0) printf(RED "You lost all lives.\n" RESET);

        printf("\nPlay Again? (y/n): ");
        scanf(" %c", &again);
        getchar();

    } while (again == 'y' || again == 'Y');

    return 0;
}
