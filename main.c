#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "array.h"
#include <SDL3/SDL.h>
#include "classes.h"

// functions
void init();
int loop(SDL_Renderer* renderer);
void lock_face();
Shape choose_random_shape();
Face make_face();
void spawn_face(Face* face);
void spawn_tetrimony();
void rotate_clockwise(Face* face);
void rotate_counter_clockwise(Face* face);
void render_board(SDL_Renderer* renderer);
void render_active_face(SDL_Renderer* renderer);
int can_move(int drow, int dcol);
int bottom_colliding();

#define MAX16_INT 65535

#define BOARD_WIDTH 16
#define BOARD_HEIGHT 32

#define CELL_SIZE 20

uint16_t board[BOARD_HEIGHT] = {0};

Face* faces;
int total_faces;

Face* active_face;



void init(){
    faces = NULL;
    total_faces = 0;
    return;
}

static inline int highest_set_bit(uint8_t x) { // -1 if none
    for (int i = 3; i >= 0; --i) if ((x >> i) & 1) return i;
    return -1;
}
static inline int lowest_set_bit(uint8_t x) { //  4 if none
    for (int i = 0; i < 4; ++i) if ((x >> i) & 1) return i;
    return 4;
}

static void normalize_shape(Shape* s) {
    // 1) find top_row and min_col across all rows
    int top_row = 4;              // first non-empty row index
    int bottom_row = -1;          // last  non-empty row index
    int min_col = 4;
    int max_col = -1;

    for (int r = 0; r < 4; ++r) {
        uint8_t row = s->pattern[r];
        if (row) {
            if (top_row == 4) top_row = r;
            bottom_row = r;
            int lo = lowest_set_bit(row);
            int hi = highest_set_bit(row);
            if (lo < min_col) min_col = lo;
            if (hi > max_col) max_col = hi;
        }
    }

    // Empty shape guard (shouldn't happen in Tetris)
    if (top_row == 4) {
        s->max_width = s->max_height = 0;
        for (int r = 0; r < 4; ++r) s->pattern[r] = 0;
        return;
    }

    // 2) shift up by top_row and right by min_col (i.e., move left)
    uint8_t tmp[4] = {0,0,0,0};
    int out_r = 0;
    for (int r = top_row; r <= bottom_row; ++r, ++out_r) {
        tmp[out_r] = s->pattern[r] >> min_col;
    }
    for (int r = out_r; r < 4; ++r) tmp[r] = 0; // clear rest
    for (int r = 0; r < 4; ++r) s->pattern[r] = tmp[r];

    // 3) recompute bounds from bit indices (NOT count-of-ones)
    s->max_height = bottom_row - top_row + 1;      // number of used rows
    s->max_width  = max_col - min_col + 1;         // width from columns used
}


void print_board(){
    printf("Board:\n");
    for (int i=0; i<BOARD_HEIGHT; i++){
        for (int j=0; j<BOARD_WIDTH; j++){
            uint8_t bit = (board[i] >> j) & 1;
            printf("%d", bit);
        }
        printf("\n");
    }
    printf("\n");
}


void rotate_clockwise(Face* face){
    // 1) rotate bits
    uint8_t rotated[4] = {0};
    for (int row=0; row<4; ++row){
        for (int col=0; col<4; ++col){
            if ((face->shape.pattern[row] >> col) & 1){
                rotated[col] |= (1 << (3-row));
            }
        }
    }
    uint8_t backup[4];
    for (int i=0;i<4;++i) backup[i] = face->shape.pattern[i];
    for (int i=0;i<4;++i) face->shape.pattern[i] = rotated[i];

    // 2) normalize to top-left and recompute bounds
    normalize_shape(&face->shape);

    // 3) test collisions; if blocked, try simple wall kicks (optional)
    if (!can_move(0,0)) {
        // simple kicks: try left, right, up (you can expand later)
        if      (can_move(0,-1)) active_face->col -= 1;
        else if (can_move(0, 1)) active_face->col += 1;
        else if (can_move(-1,0)) active_face->row -= 1;
        else {
            // 4) revert rotation if no kick works
            for (int i=0;i<4;++i) face->shape.pattern[i] = backup[i];
            normalize_shape(&face->shape); // restore bounds
        }
    }
}


void lock_face(){
    for (int i=0;i<active_face->shape.max_height;i++){
        for (int j=0; j<active_face->shape.max_width; j++){
            if ((active_face->shape.pattern[i] >> j) & 1){
                uint16_t row = active_face->shape.pattern[i] << (active_face->col);
                board[active_face->row+i] = board[active_face->row+i] | row;
            }
        }
    }
    
    for (int i=0; i<BOARD_HEIGHT; i++){
        if (board[i] == MAX16_INT){
            for (int j=i; j>0; j--){
                board[j] = board[j-1];
            }
        }
    }
    print_board();
    active_face = NULL;
    spawn_tetrimony();
    return;
}

void spawn_tetrimony(){
    active_face = malloc(sizeof(Face));
    if (!active_face) {
        printf("Error: Failed to allocate memory\n");
        return;
    }
    *active_face = make_face();
    spawn_face(active_face);
}

Shape choose_random_shape(){
    printf("Choosing the shape...\n");
    uint8_t shape_id = (uint8_t)(rand())%7 + 1;
    Shape shape;

    switch (shape_id){
        case 1:
            array_duplicate(&T_pattern, sizeof(T_pattern)/sizeof(T_pattern[0]), sizeof(T_pattern[0]), &shape.pattern);
            shape.max_width = 3;
            shape.max_height = 2;
            printf("Chose T\n");
            break;
        case 2:
            array_duplicate(&L_pattern, sizeof(L_pattern)/sizeof(L_pattern[0]), sizeof(L_pattern[0]), &shape.pattern);
            shape.max_width = 3;
            shape.max_height = 2;
            printf("Chose L\n");
            break;
        case 3:
            array_duplicate(&J_pattern, sizeof(J_pattern)/sizeof(J_pattern[0]), sizeof(J_pattern[0]), &shape.pattern);
            printf("Chose J\n");
            shape.max_width = 3;
            shape.max_height = 2;
            break;
        case 4:
            array_duplicate(&Z_pattern, sizeof(Z_pattern)/sizeof(Z_pattern[0]), sizeof(Z_pattern[0]), &shape.pattern);
            printf("Chose I\n");
            shape.max_width = 4;
            shape.max_height = 2;
            break;
        case 5:
            array_duplicate(&reverse_Z_pattern, sizeof(reverse_Z_pattern)/sizeof(reverse_Z_pattern[0]), sizeof(reverse_Z_pattern[0]), &shape.pattern);
            printf("Chose I\n");
            shape.max_width = 4;
            shape.max_height = 2;
            break;
        case 6:
            array_duplicate(&square_pattern, sizeof(square_pattern)/sizeof(square_pattern[0]), sizeof(square_pattern[0]), &shape.pattern);
            printf("Chose I\n");
            shape.max_width = 2;
            shape.max_height = 2;
            break;
        case 7:
            array_duplicate(&I_pattern, sizeof(I_pattern)/sizeof(I_pattern[0]), sizeof(I_pattern[0]), &shape.pattern);
            printf("Chose I\n");
            shape.max_width = 1;
            shape.max_height = 4;
            break;
        default:
            printf("shape id does not exist\n");
            return shape;
            break;
    }

    printf("Chosing colors...\n");
    shape.color.r = rand()%256;
    shape.color.g = rand()%256;
    shape.color.b = rand()%256;

    return shape;
}

Face make_face(){
    printf("Making the face...\n");
    Face face;
    face.shape = choose_random_shape();
    printf("Made the shape\n");

    return face;
}

void spawn_face(Face* face){
    printf("Spawning face...\n");
    /*
        shape     board    board
        0b1110 || 0b0000 = 0b1110 : Good
        0b0100 || 0b0100 = 0b0100 : Bad
        0b0000 || 0b0000 = 0b0000 
        0b0000 || 0b0000 = 0b0000

        0b1010 >> 0 = 0b1010 & 0b0001 = 0 // value of the first bit haha
        0b1010 << 0 = 0b1010 & 0b1000 = 0 // value of the fourth bit :)
    */

    face->col = 0;
    face->row = 0;


    for (int i=0; i<4; i++){
        if (board[i] != 0){
            printf("Error: Spawning area is not clear\n");
            return;
        }
        //board[i] = board[i] | face->shape.pattern[i-face->row];
    }
    

    //active_face = face;
    return;
}

void render_board(SDL_Renderer* renderer){

    Color c = {100,100,100};
    SDL_SetRenderDrawColor(renderer, (Uint8)c.r, (Uint8)c.g, (Uint8)c.b, 255);
    
    for (int j=0; j<BOARD_HEIGHT; j++){
        for (int k=0; k<BOARD_WIDTH; k++){
            if ((board[j] >> k) & 1) {
                SDL_FRect rect = {20+k*CELL_SIZE, 20+j*CELL_SIZE, CELL_SIZE, CELL_SIZE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 150, 50, 255);
    SDL_FRect rect = {20, 20, BOARD_WIDTH*CELL_SIZE, BOARD_HEIGHT*CELL_SIZE};
    SDL_RenderRect(renderer, &rect);
}


void render_active_face(SDL_Renderer* renderer){
    if (active_face != NULL){
        Color c = active_face->shape.color;
        SDL_SetRenderDrawColor(renderer, (Uint8)c.r, (Uint8)c.g, (Uint8)c.b, 255);
        //printf("rendering face.\n");
        for (int i=0; i<4; i++){
            for (int j=0; j<4; j++){
                if ((active_face->shape.pattern[i] >> j) & 1){
                    int x = (active_face->col+j) * CELL_SIZE + 20;
                    int y = (active_face->row+i) * CELL_SIZE + 20;
                    SDL_FRect rect = { x, y, CELL_SIZE, CELL_SIZE };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }
}

int can_move(int drow, int dcol) {
    if (!active_face) return 0;

    for (int r = 0; r < active_face->shape.max_height; r++) {
        uint16_t row_mask = active_face->shape.pattern[r];

        for (int c = 0; c < active_face->shape.max_width; c++) {
            if ((row_mask >> c) & 1) {  // This cell is occupied by the shape
                int new_r = active_face->row + r + drow;
                int new_c = active_face->col + c + dcol;

                // Out of bounds check
                if (new_r < 0 || new_r >= BOARD_HEIGHT || new_c < 0 || new_c >= BOARD_WIDTH) {
                    return 0;
                }

                // Collision with board check
                if ((board[new_r] >> new_c) & 1) {
                    return 0;
                }
            }
        }
    }

    return 1; // no collisions
}

int bottom_colliding(){
    if (active_face){
        for (int r = 0; r < active_face->shape.max_height; r++) {
            uint16_t row_mask = active_face->shape.pattern[r];

            for (int c = 0; c < active_face->shape.max_width; c++) {
                if ((row_mask >> c) & 1) {  // This cell is occupied by the shape
                    int new_r = active_face->row + r + 1;
                    int new_c = active_face->col + c;

                    // Collision with board check
                    if ((board[new_r] >> new_c) & 1) {
                        return 1;
                    }

                    if (new_r < 0 || new_r >= BOARD_HEIGHT){
                        return 1;
                    }
                }
            }
        }
        return 0;
    }
    return 0;
}

int loop(SDL_Renderer* renderer) {
    int quit = 0;
    Uint32 last_tick = SDL_GetTicks();
    Uint32 fall_delay = 500; // ms between automatic falls
    Uint32 last_fall = SDL_GetTicks();

    while (!quit) {
        // 1. Handle input
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = 1;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_SPACE:
                            if (!active_face) spawn_tetrimony();
                            break;
                        case SDLK_RIGHT:
                            if (active_face && can_move(0, 1))
                                active_face->col++;
                            break;
                        case SDLK_LEFT:
                            if (active_face && can_move(0, -1))
                                active_face->col--;
                            break;
                        case SDLK_DOWN:
                            if (active_face && can_move(1, 0))
                                active_face->row++;
                            break;
                        case SDLK_C:
                            if (active_face){
                                rotate_clockwise(active_face);
                            }
                    }
                    break;
            }
        }

        // 2. Update game (automatic falling)
        Uint32 now = SDL_GetTicks();
        if (now - last_fall >= fall_delay) {
            if (active_face && can_move(1, 0)) {
                active_face->row++;
            } else if (bottom_colliding()) {
                lock_face();
            }
            last_fall = now;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 3. Render
        render_board(renderer);
        render_active_face(renderer);

        SDL_RenderPresent(renderer);

        // 4. Control frame rate
        SDL_Delay(16); // ~60 fps
    }

    return 0;
}


int main(int argc, char* argv[]){
    SDL_Window *window;
    int done = 0;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Tetris", BOARD_WIDTH*CELL_SIZE+240, BOARD_HEIGHT*CELL_SIZE+40, SDL_WINDOW_OPENGL);

    if (window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    spawn_tetrimony();

    while (!done){
        
        if (!loop(renderer)) done = 1;
        
    }

    free(faces);
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}