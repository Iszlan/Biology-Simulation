#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define POPULATION 50      // Smaller population
#define PERSON_RADIUS 10    // Larger person size
#define INFECTION_RADIUS 10
#define BETA 0.9            // Infection probability
#define RECOVERY_TIME 300   // Time steps to recover
#define FRAMES 10000         // Simulation steps

#define RASH_SPOT_RADIUS 3  // Rash spot size

// Declare colors for different stages
SDL_Color COLOR_INCUBATION = {169, 169, 169, 255};  // Grey (Incubation period)
SDL_Color COLOR_PRODROMAL = {255, 223, 0, 255};    // Yellow (Prodromal period)
SDL_Color COLOR_RASH = {255, 0, 0, 255};           // Red (Rash stage)
SDL_Color COLOR_RECOVERY = {0, 255, 0, 255};       // Green (Recovered)

// Structure to define each person
typedef struct {
    float x, y;         // Position
    float dx, dy;       // Movement vector
    int state;          // 0: Susceptible, 1: Infected, 2: Recovered
    int infection_time; // Time since infected
    int alive;          // 1: Alive, 0: Dead
} Person;

void initialise_population(Person *people) {
    for (int i = 0; i < POPULATION; i++) {
        people[i].x = rand() % WINDOW_WIDTH;
        people[i].y = rand() % WINDOW_HEIGHT;
        people[i].dx = ((rand() % 3) - 1) * 2.0;  // Random small movement
        people[i].dy = ((rand() % 3) - 1) * 4.5;
        people[i].state = 0; // Start as susceptible
        people[i].infection_time = 0;
        people[i].alive = 1; // Start as alive
    }
    // Initialise patient zero
    people[0].state = 1;
}

void update_population(Person *people, int frame) {
    for (int i = 0; i < POPULATION; i++) {
        if (!people[i].alive) {
            continue;
        }

        // Update position
        people[i].x += people[i].dx;
        people[i].y += people[i].dy;

        // Boundary reflection
        if (people[i].x <= 0 || people[i].x >= WINDOW_WIDTH) people[i].dx *= -1;
        if (people[i].y <= 0 || people[i].y >= WINDOW_HEIGHT) people[i].dy *= -1;

        // Process infection recovery or death
        if (people[i].state == 1) {
            people[i].infection_time++;
            if (people[i].infection_time >= RECOVERY_TIME) {
                // 50% chance of dying if in the red stage
                if (((float)rand() / RAND_MAX) < 0.5) {
                    people[i].alive = 0; // Person dies
                } else {
                    people[i].state = 2; // Recovered
                }
            }
        }

        // Infection spread
        if (people[i].state == 1) {
            for (int j = 0; j < POPULATION; j++) {
                if (people[j].state == 0 && people[j].alive) {
                    float distance = SDL_sqrtf((people[i].x - people[j].x) * (people[i].x - people[j].x) +
                                               (people[i].y - people[j].y) * (people[i].y - people[j].y));
                    if (distance <= INFECTION_RADIUS && ((float)rand() / RAND_MAX) < BETA) {
                        people[j].state = 1; // Infect
                        people[j].infection_time = 0;
                    }
                }
            }
        }
    }
}

void draw_population(SDL_Renderer *renderer, Person *people) {
    for (int i = 0; i < POPULATION; i++) {
        if (!people[i].alive) {
            continue;
        }

        SDL_Color person_color;

        // Assign color based on state
        if (people[i].state == 0) {
            person_color = COLOR_INCUBATION; // Grey for Incubation (not infected yet)
        }
        else if (people[i].state == 1) {
            if (people[i].infection_time < RECOVERY_TIME / 2) {
                person_color = COLOR_PRODROMAL; // Yellow for Prodromal stage
            } else {
                person_color = COLOR_RASH; // Red for Rash stage
            }
        }
        else if (people[i].state == 2) {
            person_color = COLOR_RECOVERY; // Green for Recovery
        }

        // Set the person's color
        SDL_SetRenderDrawColor(renderer, person_color.r, person_color.g, person_color.b, person_color.a);

        // Draw the main circle representing the person
        for (int w = 0; w < PERSON_RADIUS * 2; w++) {
            for (int h = 0; h < PERSON_RADIUS * 2; h++) {
                int dx = PERSON_RADIUS - w; // Horizontal offset
                int dy = PERSON_RADIUS - h; // Vertical offset
                if ((dx * dx + dy * dy) <= (PERSON_RADIUS * PERSON_RADIUS)) {
                    SDL_RenderDrawPoint(renderer, people[i].x + dx, people[i].y + dy);
                }
            }
        }

        // If the person is infected, draw rash spots
        if (people[i].state == 1) {
            // Add rash pattern to infected people (bigger spots and more identifiable color)
            SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255); // Dark Red color for rash spots
            for (int j = 0; j < 8; j++) {  // Increase the number of rash spots
                // Randomise the positions of rash spots within the person’s radius
                int spot_x = rand() % (PERSON_RADIUS * 2) - PERSON_RADIUS;
                int spot_y = rand() % (PERSON_RADIUS * 2) - PERSON_RADIUS;
                // Make sure the spot is within the boundary of the person (within the main circle)
                if (spot_x * spot_x + spot_y * spot_y <= PERSON_RADIUS * PERSON_RADIUS) {
                    // Draw the rash as a bigger spot
                    for (int w = -RASH_SPOT_RADIUS; w < RASH_SPOT_RADIUS; w++) {
                        for (int h = -RASH_SPOT_RADIUS; h < RASH_SPOT_RADIUS; h++) {
                            if ((w * w + h * h) <= (RASH_SPOT_RADIUS * RASH_SPOT_RADIUS)) {
                                SDL_RenderDrawPoint(renderer, people[i].x + spot_x + w, people[i].y + spot_y + h);
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error initialising SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Measles Spread Simulation",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (!window) {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Person people[POPULATION];
    initialise_population(people);

    int running = 1, frame = 0;
    SDL_Event event;

    while (running && frame < FRAMES) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Update and draw population
        update_population(people, frame);
        draw_population(renderer, people);

        // Present the renderer
        SDL_RenderPresent(renderer);

        // Frame control
        SDL_Delay(16); // ~60 FPS
        frame++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
