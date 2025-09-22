#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <array.h>
#include <SDL3/SDL.h>
#include <classes.h>
#include <text.h>

typedef enum {
    STATE_START,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
}GameState;

static GameState g_state = STATE_START;

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
void render_UI(SDL_Renderer* renderer);
int can_move(int drow, int dcol);
int bottom_colliding();
int start(SDL_Renderer* renderer);
int pause(SDL_Renderer* renderer);
char* get_current_time();
char* intToStr(int n);
void draw_text(SDL_Renderer* renderer, const char* string, int x, int y, Color cl, int scale);
void restart(SDL_Renderer* renderer);
int game_over(SDL_Renderer* renderer);
void render_game_over(SDL_Renderer* renderer);
void render_start(SDL_Renderer* renderer);
void render_paused(SDL_Renderer* renderer);
void render_playing(SDL_Renderer* renderer);

#define MAX16_INT 65535

#define BOARD_WIDTH 16
#define BOARD_HEIGHT 32

#define CELL_SIZE 20

uint16_t board[BOARD_HEIGHT] = {0};

uint64_t score = 0;
char* score_str = NULL;

Face* active_face;

uint8_t* font_table[138] = {
    ['A'] = font_A,
    ['B'] = font_B,
    ['C'] = font_C,
    ['D'] = font_D,
    ['E'] = font_E,
    ['F'] = font_F,
    ['G'] = font_G,
    ['H'] = font_H,
    ['I'] = font_I,
    ['J'] = font_J,
    ['K'] = font_K,
    ['L'] = font_L,
    ['M'] = font_M,
    ['N'] = font_N,
    ['O'] = font_O,
    ['P'] = font_P,
    ['Q'] = font_Q,
    ['R'] = font_R,
    ['S'] = font_S,
    ['T'] = font_T,
    ['U'] = font_U,
    ['V'] = font_V,
    ['W'] = font_W,
    ['X'] = font_X,
    ['Y'] = font_Y,
    ['Z'] = font_Z,

    ['0'] = font_0,
    ['1'] = font_1,
    ['2'] = font_2,
    ['3'] = font_3,
    ['4'] = font_4,
    ['5'] = font_5,
    ['6'] = font_6,
    ['7'] = font_7,
    ['8'] = font_8,
    ['9'] = font_9,

    [30] = font_0,
    [31] = font_1,
    [32] = font_2,
    [33] = font_3,
    [34] = font_4,
    [35] = font_5,
    [36] = font_6,
    [37] = font_7,
    [38] = font_8,
    [39] = font_9,

    ['!'] = font_exclamation,
    ['?'] = font_question,
    ['.'] = font_period,
    [','] = font_comma,
    ['-'] = font_dash,
    ['+'] = font_plus,
    ['*'] = font_star,
    ['/'] = font_slash,
    [':'] = font_colon,
    [' '] = font_space
};

void render_game_over(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // 3. Render
    render_board(renderer);
    render_active_face(renderer);
    render_UI(renderer);
    Color color = {255,0,0};
    int scale = 4;
    int x = 20+(BOARD_WIDTH/2)*CELL_SIZE-(9*CHAR_SPACE_*scale)/2;
    draw_text(renderer, "GAME OVER", x, 20+(BOARD_HEIGHT/2)*CELL_SIZE, color, scale);
    draw_text(renderer, "PRESS ANY KEY TO RESTART", 20+(BOARD_WIDTH/2)*CELL_SIZE-(24*CHAR_SPACE_*2)/2, 60+(BOARD_HEIGHT/2)*CELL_SIZE, color, 2);
    SDL_RenderPresent(renderer);
}

void render_start(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 3. Render
    render_board(renderer);
    render_active_face(renderer);
    render_UI(renderer);
    Color color = {255,255,255};
    int scale = 2;
    int x = 20+(BOARD_WIDTH/2)*CELL_SIZE-(22*CHAR_SPACE_*scale)/2;
    draw_text(renderer, "PRESS ANY KEY TO START", x, 20+(BOARD_HEIGHT/2)*CELL_SIZE, color, scale);
    SDL_RenderPresent(renderer);
}

void render_paused(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 3. Render
    render_board(renderer);
    render_active_face(renderer);
    render_UI(renderer);
    Color color = {255,255,255};
    int scale = 4;
    int x = 20+(BOARD_WIDTH/2)*CELL_SIZE-(6*CHAR_SPACE_*scale)/2;
    draw_text(renderer, "PAUSED", x, 20+(BOARD_HEIGHT/2)*CELL_SIZE, color, scale);
    SDL_RenderPresent(renderer);
}

int game_over(SDL_Renderer* renderer){
    while (1){
        render_game_over(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key){
                        case SDLK_SPACE:
                            return 1;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
    }
}

int start(SDL_Renderer* renderer){
    for (int i=0; i<BOARD_HEIGHT; i++){
        board[i] = 0;
    }
    free(active_face);
    free(score_str);
    score = 0;
    score_str = intToStr(score);
    while (1){
        render_start(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    return 0;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    return 1;
                    break;
            }
        }

        
    }
}


int pause(SDL_Renderer* renderer){
    while (1){
        render_paused(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    return 0;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key){
                        case SDLK_SPACE:
                            return 1;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
    }
}

char* intToStr(int n){
    char* str = NULL;
    int len = 0;
    int temp = n;
    while (temp/10 != 0){
        len++;
        temp /= 10;
    }
    len++;
    str = (char*)malloc(sizeof(char)*(len+1));
    if (str == NULL){
        printf("Failed to allocate memory\n");
        return "\0";
    }
    str[len] = '\0';
    while(len-1 >= 0){
        int digit = n%10;
        char c = digit+'0';
        str[len-1] = c;
        len--;
        n /= 10;
    }

    return str;
}

void draw_text(SDL_Renderer* renderer, const char* string, int x, int y, Color cl, int scale) {
    SDL_SetRenderDrawColor(renderer, (Uint8)cl.r, (Uint8)cl.g, (Uint8)cl.b, 255);

    int len = strlen(string);
    for (int i = 0; i < len; i++) {
        char ch = string[i];

        // Get bitmap for this character
        uint8_t* font_char = font_table[(int)ch];
        if (!font_char) continue; // skip unsupported characters

        for (int r = 0; r < CHAR_HEIGHT_; r++) {
            for (int c = 0; c < CHAR_WIDTH_; c++) {
                if ((font_char[r] >> (CHAR_WIDTH_ - 1 - c)) & 1) {
                    int xpos = x + c*scale + i*scale * CHAR_SPACE_;
                    int ypos = y + r*scale;

                    SDL_FRect rect = {xpos, ypos, scale, scale};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }
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
            score += 10;
            score_str = intToStr(score);
        }
    }
    //print_board();
    free(active_face);
    spawn_tetrimony();
    return;
}

void spawn_tetrimony(){
    for (int i=0; i<4; i++){
        if (board[i] != 0){
            g_state = STATE_GAME_OVER;
            break;
        }
    }

    active_face = malloc(sizeof(Face));
    if (!active_face) {
        printf("Error: Failed to allocate memory\n");
        return;
    }
    *active_face = make_face();
    spawn_face(active_face);
}

Shape choose_random_shape(){
    uint8_t shape_id = (uint8_t)(rand())%10 + 1;
    Shape shape;

    switch (shape_id){
        case 1:
            array_duplicate(&T_pattern, sizeof(T_pattern)/sizeof(T_pattern[0]), sizeof(T_pattern[0]), &shape.pattern);
            shape.max_width = 3;
            shape.max_height = 2;
            break;
        case 2:
            array_duplicate(&L_pattern, sizeof(L_pattern)/sizeof(L_pattern[0]), sizeof(L_pattern[0]), &shape.pattern);
            shape.max_width = 3;
            shape.max_height = 2;
            break;
        case 3:
            array_duplicate(&J_pattern, sizeof(J_pattern)/sizeof(J_pattern[0]), sizeof(J_pattern[0]), &shape.pattern);
            shape.max_width = 3;
            shape.max_height = 2;
            break;
        case 4:
            array_duplicate(&Z_pattern, sizeof(Z_pattern)/sizeof(Z_pattern[0]), sizeof(Z_pattern[0]), &shape.pattern);
            shape.max_width = 4;
            shape.max_height = 2;
            break;
        case 5:
            array_duplicate(&reverse_Z_pattern, sizeof(reverse_Z_pattern)/sizeof(reverse_Z_pattern[0]), sizeof(reverse_Z_pattern[0]), &shape.pattern);
            shape.max_width = 4;
            shape.max_height = 2;
            break;
        case 6:
            array_duplicate(&square_pattern, sizeof(square_pattern)/sizeof(square_pattern[0]), sizeof(square_pattern[0]), &shape.pattern);
            shape.max_width = 2;
            shape.max_height = 2;
            break;
        case 7:
            array_duplicate(&I_pattern, sizeof(I_pattern)/sizeof(I_pattern[0]), sizeof(I_pattern[0]), &shape.pattern);
            shape.max_width = 1;
            shape.max_height = 4;
            break;
        case 8:
            array_duplicate(&dot_pattern, sizeof(dot_pattern)/sizeof(dot_pattern[0]), sizeof(dot_pattern[0]), &shape.pattern);
            shape.max_width = 1;
            shape.max_height = 1;
            break;
        case 9:
            array_duplicate(&mini_L_pattern, sizeof(mini_L_pattern)/sizeof(mini_L_pattern[0]), sizeof(mini_L_pattern[0]), &shape.pattern);
            shape.max_width = 2;
            shape.max_height = 2;
            break;
        case 10:
            array_duplicate(&mini_J_pattern, sizeof(mini_J_pattern)/sizeof(mini_J_pattern[0]), sizeof(mini_J_pattern[0]), &shape.pattern);
            shape.max_width = 2;
            shape.max_height = 2;
            break;
        default:
            printf("shape id does not exist\n");
            return shape;
            break;
    }

    shape.color.r = 50 + rand()%206;
    shape.color.g = 50 + rand()%206;
    shape.color.b = 50 + rand()%206;

    return shape;
}

Face make_face(){
    Face face;
    face.shape = choose_random_shape();

    return face;
}

void spawn_face(Face* face){
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

void render_UI(SDL_Renderer* renderer){
    Color color = {255,255,94};
    draw_text(renderer, "TETRIS", 40+BOARD_WIDTH*CELL_SIZE, 20, color, 5);
    color.r = 255;
    color.g = 255;
    color.b = 255;
    draw_text(renderer, get_current_time(), 20, BOARD_HEIGHT*CELL_SIZE+24, color, 2);
    draw_text(renderer, "SCORE", BOARD_WIDTH*CELL_SIZE+120-(5*CHAR_SPACE_*2)/2, (BOARD_HEIGHT*CELL_SIZE/2)-CHAR_HEIGHT_*2, color, 2);
    draw_text(renderer, score_str, BOARD_WIDTH*CELL_SIZE+40, (BOARD_HEIGHT*CELL_SIZE/2)+CHAR_HEIGHT_*2, color, 2);
    draw_text(renderer, "C: ROTATE", BOARD_WIDTH*CELL_SIZE+40, (BOARD_HEIGHT*CELL_SIZE/2)+CHAR_HEIGHT_*10*2, color, 2);
    draw_text(renderer, "ARROW KEYS: MOVE", BOARD_WIDTH*CELL_SIZE+40, (BOARD_HEIGHT*CELL_SIZE/2)+CHAR_HEIGHT_*12*2, color, 2);
    draw_text(renderer, "MADE BY:", 40+BOARD_WIDTH*CELL_SIZE, BOARD_HEIGHT*CELL_SIZE-20, color, 2);
    color.r = 144;
    color.g = 165;
    color.b = 255;
    draw_text(renderer, "DRAGONDEV", 40+BOARD_WIDTH*CELL_SIZE, BOARD_HEIGHT*CELL_SIZE, color, 2);
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
        switch (g_state){
            case STATE_GAME_OVER:
                if (game_over(renderer)){
                    g_state = STATE_START;
                    if (start(renderer)){
                        g_state = STATE_PLAYING;
                        fall_delay = 500;
                        spawn_tetrimony();
                    }else{
                        quit = 1;
                    }
                }
                break;
            case STATE_START:
                if (start(renderer)){
                    g_state = STATE_PLAYING;
                    spawn_tetrimony();
                }else{
                    quit = 1;
                }
                break;
            case STATE_PAUSED:
                if (pause(renderer)){
                    g_state = STATE_PLAYING;
                }else{
                    quit = 1;
                }
                break;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = 1;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    switch (g_state){
                        case STATE_PLAYING:
                            switch (event.key.key) {
                                case SDLK_ESCAPE:
                                    quit = 1;
                                    break;
                                case SDLK_SPACE:
                                    g_state = STATE_PAUSED;
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
                    break;
                default:
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
                if (fall_delay > 50){
                    fall_delay -= 5;
                }
            }
            last_fall = now;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 3. Render
        render_board(renderer);
        render_active_face(renderer);
        render_UI(renderer);

        SDL_RenderPresent(renderer);

        // 4. Control frame rate
        SDL_Delay(16); // ~60 fps
    }

    return 0;
}




char* get_current_time(){
    time_t current_time;
    current_time = time(NULL);
    struct tm *cur_time_ptr = localtime(&current_time);
    char* str = asctime(cur_time_ptr);
    int size = strlen(str);
    for (int i=0; i<size; i++){
        if (str[i] >= 97 && str[i] <= 122){
            str[i] -= 32;
        }
    }
    return str;
}

int main(int argc, char* argv[]){
    printf("Made by:\n\tDragonDev\n\nLibraries used:\n\tSDL3 - rendering & input handling\n\tminiaudio - audio playback\n\nIcon and music are copyrighted materials from Tetris 99 and Tetris (Original) respectively\nand belong to their respective owners.\nUsed here for educational and non-commercial purposes only.\n\n2025 DragonDev. All rights reserved.");
    SDL_Window *window;
    int done = 0;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Tetris", BOARD_WIDTH*CELL_SIZE+240, BOARD_HEIGHT*CELL_SIZE+40, SDL_WINDOW_OPENGL);

    if (window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface* icon = SDL_LoadBMP("res/icon.bmp");  // Must be BMP or load with SDL_image
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon); // free after setting
    } else {
        printf("Failed to load icon: %s\n", SDL_GetError());
    }


    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    ma_sound sound;
    if (ma_sound_init_from_file(&engine, "res/Tetris.mp3", MA_SOUND_FLAG_LOOPING, NULL, NULL, &sound) != MA_SUCCESS) {
        printf("Failed to load sound\n");
        return -1;
    }

    // Play it
    ma_sound_start(&sound);

    while (!done){
        
        if (!loop(renderer)) done = 1;
        
    }

    
    SDL_DestroyWindow(window);
    SDL_Quit();
    ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);

    return 0;
}