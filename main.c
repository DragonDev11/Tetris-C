#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "array.h"
#include <SDL3/SDL.h>
#include "classes.h"

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

Shape choose_random_shape(){
    printf("Choosing the shape...\n");
    uint8_t shape_id = (uint8_t)(rand())%4 + 1;
    Shape shape;

    switch (shape_id){
        case 1:
            array_duplicate(&T_pattern, sizeof(T_pattern)/sizeof(T_pattern[0]), sizeof(T_pattern[0]), &shape.pattern);
            printf("Chose T\n");
            break;
        case 2:
            array_duplicate(&L_pattern, sizeof(L_pattern)/sizeof(L_pattern[0]), sizeof(L_pattern[0]), &shape.pattern);
            printf("Chose L\n");
            break;
        case 3:
            array_duplicate(&J_pattern, sizeof(J_pattern)/sizeof(J_pattern[0]), sizeof(J_pattern[0]), &shape.pattern);
            printf("Chose J\n");
            break;
        case 4:
            array_duplicate(&I_pattern, sizeof(I_pattern)/sizeof(I_pattern[0]), sizeof(I_pattern[0]), &shape.pattern);
            printf("Chose I\n");
            break;
        default:
            printf("shape id not handeled\n");
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

    for (int i=0; i<4; i++){
        if (board[i] != 0){
            printf("Error: Spawning area is not clear\n");
            return;
        }
    }

    face->vertices = NULL;
    Vertex* tmp = NULL;
    int total_vertices = 0;
    int current_vertex = 0;

    printf("Creating the face's vertices...\n");

    for (int i=0; i<4; i++){

        board[i] = (board[i] | face->shape.pattern[i]);
        printf("\nByte %d: %d\n", i, board[i]);
        for (int j=0; j<4; j++){

            uint8_t bit = (board[i] >> j) & 0b0001;
            printf("bit %d\n", bit);
            if (bit == 1){
                tmp = (Vertex*)realloc(face->vertices, sizeof(Vertex) * (total_vertices+4));
                if (tmp == NULL){
                    printf("Failed to reallocate memory\n");
                    free(face->vertices);
                    return;
                }

                face->vertices = tmp;

                face->vertices[current_vertex].x = j*CELL_SIZE;
                face->vertices[current_vertex].y = i*CELL_SIZE;
                current_vertex++;
                
                face->vertices[current_vertex].x = face->vertices[current_vertex-1].x + CELL_SIZE;
                face->vertices[current_vertex].y = face->vertices[current_vertex-1].y;
                current_vertex++;

                face->vertices[current_vertex].x = face->vertices[current_vertex-1].x;
                face->vertices[current_vertex].y = face->vertices[current_vertex-1].y + CELL_SIZE;
                current_vertex++;

                face->vertices[current_vertex].x = face->vertices[current_vertex-1].x - CELL_SIZE;
                face->vertices[current_vertex].y = face->vertices[current_vertex-1].y;

                face->vertices[current_vertex-3].next = &face->vertices[current_vertex-2];
                face->vertices[current_vertex-2].next = &face->vertices[current_vertex-1];
                face->vertices[current_vertex-1].next = &face->vertices[current_vertex];
                face->vertices[current_vertex].next = &face->vertices[current_vertex-3];

                current_vertex++;
                total_vertices += 4;
            }
        }
    }
    
    face->total_vertices = total_vertices;

    faces = (Face*)realloc(faces, sizeof(Face)*(total_faces+1));
    if (faces == NULL){
        printf("Failed to reallocate memory\n");
        return;
    }

    faces[total_faces] = *face;
    total_faces++;
    return;
}

void render_faces(SDL_Renderer* renderer){
    printf("Rendering faces...\n");
    for (int i=0; i<total_faces; i++){
        //printf("Rendering face %d...\n", i);
        Color face_color = faces[i].shape.color;
        SDL_SetRenderDrawColor(renderer, (Uint8)face_color.r, (Uint8)face_color.g, (Uint8)face_color.b, 255);
        for (int j=0; j<faces[i].total_vertices; j++){
            Vertex* v1 = &faces[i].vertices[j];
            Vertex* v2 = v1->next;  
            if (v2) {
                //printf("Drawing line between vertex(%.2f,%.2f) and vertex(%.2f,%.2f)...\n", v1->x, v1->y, v2->x, v2->y);
                SDL_RenderLine(renderer, v1->x, v1->y, v2->x, v2->y);
            }
        }
    }

    for (int i=0; i<4; i++){
        printf("%d\n", board[i]);
    }
}

void loop(){

    return;
}

int main(int argc, char* argv[]){
    SDL_Window *window;
    int done = 0;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Tetris", BOARD_WIDTH*CELL_SIZE, BOARD_HEIGHT*CELL_SIZE, SDL_WINDOW_OPENGL);

    if (window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    while (!done){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Event event;

        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key){
                        case SDLK_ESCAPE:
                            done = 1;
                            break;
                        case SDLK_SPACE:
                            Face face = make_face();
                            spawn_face(&face);
                            active_face = &face;
                            break;
                        case SDLK_RIGHT:
                            if (active_face != NULL){
                                for (int i=0; i<active_face->total_vertices; i++){
                                    active_face->vertices[i].x += CELL_SIZE;
                                }
                                
                            }
                            break;
                        case SDLK_LEFT:
                            if (active_face != NULL){
                                for (int i=0; i<active_face->total_vertices; i++){
                                    active_face->vertices[i].x -= CELL_SIZE;
                                }
                            }
                            break;
                        case SDLK_UP:
                            if (active_face != NULL){
                                for (int i=0; i<active_face->total_vertices; i++){
                                    active_face->vertices[i].y -= CELL_SIZE;
                                }
                            }
                            break;
                        case SDLK_DOWN:
                            if (active_face != NULL){
                                for (int i=0; i<active_face->total_vertices; i++){
                                    active_face->vertices[i].y += CELL_SIZE;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        render_faces(renderer);
        SDL_RenderPresent(renderer);
    }

    free(faces);
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}