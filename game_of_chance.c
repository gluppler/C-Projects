#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include "hacking.h"

#define DATAFILE "/var/chance.data" // File to store user data

// Custom user struct to store information about users
struct user {
    int uid;
    int credits;
    int highscore;
    char name[100];
    int (*current_game)();
};

// Function prototypes
int get_player_data();
void register_new_player();
void update_player_data();
void show_highscore();
void jackpot();
void input_name();
void print_cards(char *, char *, int);
int take_wager(int, int);
void play_the_game();
int pick_a_number();
int dealer_no_match();
int find_the_ace();
void fatal(char *);

// Global variables
struct user player; // Player struct

int main() {
    int choice, last_game = 0;
    srand(time(0)); // Seed the randomizer with the current time.

    if(get_player_data() == -1) // Try to read player data from file.
        register_new_player();  // If there is no data, register a new player.

    while(choice != 7) {
        printf("-=[ Game of Chance Menu ]=-\n");
        printf("1 - Play the Pick a Number game\n");
        printf("2 - Play the No Match Dealer game\n");
        printf("3 - Play the Find the Ace game\n");
        printf("4 - View current high score\n");
        printf("5 - Change your user name\n");
        printf("6 - Reset your account at 100 credits\n");
        printf("7 - Quit\n");
        printf("[Name: %s]\n", player.name);
        printf("[You have %u credits] -> ", player.credits);

        scanf("%d", &choice);

        if(choice < 1 || choice > 7) {
            printf("\n[!!] The number %d is an invalid selection.\n\n", choice);
        } else if (choice < 4) { // Game choice
            if(choice != last_game) {
                // Assign the game function pointer based on the choice
                player.current_game = (choice == 1) ? pick_a_number :
                                      (choice == 2) ? dealer_no_match : find_the_ace;
                last_game = choice;
            }
            play_the_game(); // Play the game.
        } else if (choice == 4) {
            show_highscore();
        } else if (choice == 5) {
            printf("\nChange user name\n");
            printf("Enter your new name: ");
            input_name();
            printf("Your name has been changed.\n\n");
        } else if (choice == 6) {
            printf("\nYour account has been reset with 100 credits.\n\n");
            player.credits = 100;
        }
    }

    update_player_data();
    printf("\nThanks for playing! Bye.\n");
    return 0;
}

// ========================== User Data Functions ==========================

// Reads player data for the current uid from the file, returns -1 if not found
int get_player_data() {
    int fd, uid = getuid(), read_bytes;
    struct user entry;

    fd = open(DATAFILE, O_RDONLY);
    if(fd == -1) return -1;

    while((read_bytes = read(fd, &entry, sizeof(struct user))) > 0) {
        if(entry.uid == uid) { // Proper uid found
            player = entry;    // Copy the entry to the player struct
            close(fd);
            return 1;
        }
    }

    close(fd);
    return -1;
}

// Registers a new user by creating a new player account and appending to the file
void register_new_player() {
    int fd;
    printf("-=-={ New Player Registration }=-=-\n");
    printf("Enter your name: ");
    input_name();

    player.uid = getuid();
    player.highscore = player.credits = 100;

    fd = open(DATAFILE, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(fd == -1) fatal("in register_new_player() while opening file");

    write(fd, &player, sizeof(struct user));
    close(fd);

    printf("\nWelcome to the Game of Chance, %s.\n", player.name);
    printf("You have been given %u credits.\n", player.credits);
}

// Updates player data (credits, high score, name) in the file
void update_player_data() {
    int fd, i, read_uid;
    char burned_byte;

    fd = open(DATAFILE, O_RDWR);
    if(fd == -1) fatal("in update_player_data() while opening file");

    read(fd, &read_uid, 4); // Read the uid from the first struct

    while(read_uid != player.uid) { // Loop until correct uid is found
        for(i = 0; i < sizeof(struct user) - 4; i++)
            read(fd, &burned_byte, 1);

        read(fd, &read_uid, 4);
    }

    write(fd, &(player.credits), 4);    // Update credits
    write(fd, &(player.highscore), 4);  // Update highscore
    write(fd, &(player.name), 100);     // Update name
    close(fd);
}

// =========================== Game Functions ==============================

// Displays the current high score and the name of the player with it
void show_highscore() {
    unsigned int top_score = 0;
    char top_name[100];
    struct user entry;
    int fd;

    printf("\n====================| HIGH SCORE |====================\n");

    fd = open(DATAFILE, O_RDONLY);
    if(fd == -1) fatal("in show_highscore() while opening file");

    while(read(fd, &entry, sizeof(struct user)) > 0) {
        if(entry.highscore > top_score) {
            top_score = entry.highscore;
            strcpy(top_name, entry.name);
        }
    }

    close(fd);
    printf("%s has the high score of %u\n", (top_score > player.highscore) ? top_name : "You",
           (top_score > player.highscore) ? top_score : player.highscore);
    printf("======================================================\n\n");
}

// Awards the jackpot for the Pick a Number game
void jackpot() {
    printf("*+*+*+*+*+* JACKPOT *+*+*+*+*+*\n");
    printf("You have won the jackpot of 100 credits!\n");
    player.credits += 100;
}

// Function to input player name, ensuring spaces are allowed
void input_name() {
    char *name_ptr, input_char = '\n';

    while(input_char == '\n')
        scanf("%c", &input_char); // Flush newline chars

    name_ptr = player.name; // Set pointer to name array

    while(input_char != '\n') {
        *name_ptr++ = input_char;
        scanf("%c", &input_char);
    }

    *name_ptr = '\0'; // Null-terminate the string
}

// Prints the 3 cards for the Find the Ace game
void print_cards(char *message, char *cards, int user_pick) {
    int i;
    printf("\n\t*** %s ***\n", message);
    printf(" \t._.\t._.\t._.\n");
    printf("Cards:\t|%c|\t|%c|\t|%c|\n\t", cards[0], cards[1], cards[2]);

    if(user_pick == -1) printf(" 1 \t 2 \t 3\n");
    else {
        for(i = 0; i < user_pick; i++)
            printf("\t");
        printf(" ^-- your pick\n");
    }
}

// Handles wagers for the games, returns -1 if wager is invalid
int take_wager(int available_credits, int previous_wager) {
    int wager, total_wager;

    printf("How many of your %d credits would you like to wager? ", available_credits);
    scanf("%d", &wager);

    if(wager < 1) {
        printf("Nice try, but you must wager a positive number!\n");
        return -1;
    }

    total_wager = previous_wager + wager;

    if(total_wager > available_credits) {
        printf("Your total wager of %d is more than you have!\n", total_wager);
        printf("You only have %d available credits, try again.\n", available_credits);
        return -1;
    }

    return wager;
}

// Plays the game by calling the current game function pointer
void play_the_game() {
    int play_again = 1;
    char selection;

    while(play_again) {
        printf("\n[DEBUG] current_game pointer @ 0x%08x\n", player.current_game);

        if(player.current_game() != -1) { // Play game if no errors
            if(player.credits > player.highscore)
                player.highscore = player.credits;

            printf("\nYou now have %u credits\n", player.credits);
            update_player_data(); // Save new credit total

            printf("Would you like to play again? (y/n) ");
            selection = '\n';
            while(selection == '\n')
                scanf("%c", &selection);

            if(selection == 'n') play_again = 0;
        } else
            play_again = 0;
    }
}

// This function is the Pick a Number game.
// It returns -1 if the player doesn't have enough credits.
int pick_a_number() {
    int pick, winning_number;

    printf("\n####### Pick a Number ######\n");
    printf("This game costs 10 credits to play. Simply pick a number\n");
    printf("between 1 and 20, and if you pick the winning number, you\n");
    printf("will win the jackpot of 100 credits!\n\n");

    winning_number = (rand() % 20) + 1; // Pick a number between 1 and 20.

    if (player.credits < 10) {
        printf("You only have %d credits. That's not enough to play!\n\n", player.credits);
        return -1; // Not enough credits to play
    }

    player.credits -= 10; // Deduct 10 credits.
    printf("10 credits have been deducted from your account.\n");
    printf("Pick a number between 1 and 20: ");
    scanf("%d", &pick);
    printf("The winning number is %d\n", winning_number);

    if (pick == winning_number)
        jackpot();
    else
        printf("Sorry, you didn't win.\n");

    return 0;
}

// This is the No Match Dealer game.
// It returns -1 if the player has 0 credits.
int dealer_no_match() {
    int i, j, numbers[16], wager = -1, match = -1;

    printf("\n::::::: No Match Dealer :::::::\n");
    printf("In this game, you can wager up to all of your credits.\n");
    printf("The dealer will deal out 16 random numbers between 0 and 99.\n");
    printf("If there are no matches among them, you double your money!\n\n");

    if (player.credits == 0) {
        printf("You don't have any credits to wager!\n\n");
        return -1;
    }

    while (wager == -1)
        wager = take_wager(player.credits, 0);

    printf("\t\t::: Dealing out 16 random numbers :::\n");
    for (i = 0; i < 16; i++) {
        numbers[i] = rand() % 100; // Pick a number between 0 and 99.
        printf("%2d\t", numbers[i]);
        if (i % 8 == 7) // Print a line break every 8 numbers.
            printf("\n");
    }

    // Check for matches
    for (i = 0; i < 15; i++) {
        for (j = i + 1; j < 16; j++) {
            if (numbers[i] == numbers[j])
                match = numbers[i];
        }
    }

    if (match != -1) {
        printf("The dealer matched the number %d!\n", match);
        printf("You lose %d credits.\n", wager);
        player.credits -= wager;
    } else {
        printf("There were no matches! You win %d credits!\n", wager);
        player.credits += wager;
    }

    return 0;
}

// This is the Find the Ace game.
// It returns -1 if the player has 0 credits.
int find_the_ace() {
    int i, ace, total_wager, invalid_choice, pick = -1, wager_one = -1, wager_two = -1;
    char choice_two, cards[3] = {'X', 'X', 'X'};

    ace = rand() % 3; // Place the ace randomly.
    printf("******* Find the Ace *******\n");
    printf("In this game, you can wager up to all of your credits.\n");
    printf("Three cards will be dealt out, two queens and one ace.\n");
    printf("If you find the ace, you will win your wager.\n");
    printf("After choosing a card, one of the queens will be revealed.\n");
    printf("At this point, you may either select a different card or\n");
    printf("increase your wager.\n\n");

    if (player.credits == 0) {
        printf("You don't have any credits to wager!\n\n");
        return -1;
    }

    while (wager_one == -1) // Loop until valid wager is made.
        wager_one = take_wager(player.credits, 0);

    print_cards("Dealing cards", cards, -1);

    // Get player's card pick
    while (pick < 1 || pick > 3) {
        printf("Select a card: 1, 2, or 3 ");
        scanf("%d", &pick);
    }
    pick--; // Adjust pick since card numbering starts at 0.

    // Reveal one of the queens
    for (i = 0; i == ace || i == pick; i++); // Find a valid queen to reveal.
    cards[i] = 'Q';
    print_cards("Revealing a queen", cards, pick);

    // Allow player to change pick or increase wager
    invalid_choice = 1;
    while (invalid_choice) {
        printf("Would you like to:\n[c]hange your pick\tor\t[i]ncrease your wager?\n");
        printf("Select c or i: ");
        choice_two = '\n';

        // Read choice
        while (choice_two == '\n')
            scanf("%c", &choice_two);

        if (choice_two == 'i') {
            invalid_choice = 0;
            while (wager_two == -1)
                wager_two = take_wager(player.credits, wager_one);
        } else if (choice_two == 'c') {
            invalid_choice = 0;
            for (i = 0; i == pick || cards[i] == 'Q'; i++); // Find the other card
            pick = i;
            printf("Your card pick has been changed to card %d\n", pick + 1);
        }
    }

    // Reveal all cards and display results
    for (i = 0; i < 3; i++)
        cards[i] = (i == ace) ? 'A' : 'Q';
    print_cards("End result", cards, pick);

    // Calculate result based on the pick
    if (pick == ace) {
        printf("You have won %d credits from your first wager\n", wager_one);
        player.credits += wager_one;
        if (wager_two != -1) {
            printf("and an additional %d credits from your second wager!\n", wager_two);
            player.credits += wager_two;
        }
    } else {
        printf("You have lost %d credits from your first wager\n", wager_one);
        player.credits -= wager_one;
        if (wager_two != -1) {
            printf("and an additional %d credits from your second wager!\n", wager_two);
            player.credits -= wager_two;
        }
    }

    return 0;
}
