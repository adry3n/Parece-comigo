//wall.png valor 0
//left_shadow_floor.png (3 variações) valor 1
//right_shadow_floor.png (3 variações) valor 2
//top_shadow_floor.png (2 variações) valor 3
//top_left_shadow_floor.png (40x40 pixels, 1 variação) valor 4
//top_right_shadow_floor.png (40x40 pixels, 1 variação) valor 5
//floor.png (6 variações) valor 6

#define _USE_MATH_DEFINES
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define KNIFE_WIDTH 64        // largura dos quadros da faca
#define KNIFE_HEIGHT 56       // altura dos quadros da faca
#define KNIFE_FRAMES 3        // número de quadros na animação
#define KNIFE_ANIM_SPEED 0.1  // velocidade da animação
#define KNIFE_RANGE 50        // alcance do ataque da faca
#define WIDTH 640
#define HEIGHT 480
#define SPRITE_SIZE 48
#define SPRITE_COLS 3
#define SPRITE_ROWS 4
#define PHRASES_PER_SET 8
#define NUM_DIALOGUE_SETS 5
#define MAP_WIDTH 1280
#define MAP_HEIGHT 960
#define NUM_GAMEPLAY_TRACKS 7 // numero de musicas durante o jogo
#define NUM_VOICE_SOUNDS 6 // Numero de vozes
#define NUM_NPCS 15
#define MIN_NPC_DISTANCE 800
#define MAX_NPC_DISTANCE 1200
#define TILE_SIZE 40          // Tamanho dos tiles
#define TILE_COLS (MAP_WIDTH / TILE_SIZE)  // 32 colunas
#define TILE_ROWS (MAP_HEIGHT / TILE_SIZE) // 24 linhas
#define CHANCE_TO_PURSUE 7    // Chance de um NPC começar a perseguir
#define LIGHT_RADIUS 125      // Raio do círculo de luz
#define AMBIENT_DARKNESS 0.95f  // Escuridão do ambiente (0.0 = claro, 1.0 = preto)

// struck das entidades
typedef struct {
    float x, y;
    int frame, movement;
    bool alive;
    bool is_target;
    ALLEGRO_BITMAP *sprite_sheet;
    float move_timer;
    float move_duration;
    int move_mode; // -1: Perseguindo; 0-3: Aleatório
    float death_timer; // Contador para a animação de morte. 0 = não morrendo. >0 = morrendo.
} Entity;

// struck para a faca
typedef struct {
    float x, y;              // Posição da faca (relativa ao jogador)
    int frame;               // Quadro da animação
    bool active;             // Se a faca ta sendo exibida
    float anim_timer;        // Temporizador para controlar a animação
    ALLEGRO_BITMAP *sprite_sheet; // Sprite da faca
} Knife;

// struck para os itens
typedef struct {
    float x, y;
    bool active;
    ALLEGRO_BITMAP *sprite;
} Item;

// struck para as telas
typedef enum {
    STATE_MAIN_MENU,
    STATE_GAMEPLAY,
    STATE_INSTRUCTIONS,
    STATE_EXITING
} GameState;

// declarações
    Entity player;
    Entity npcs[NUM_NPCS];
    Knife knife;
    Item mirror_item;
    bool game_over = false;
    bool victory = false;
    bool found_mirror = false;
    ALLEGRO_BITMAP *character_sprites[NUM_NPCS];

// matriz do mapa
    int map[TILE_ROWS][TILE_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,4,3,3,3,3,5,0,4,3,3,3,3,5,0,4,3,3,3,3,3,3,5,0,4,3,3,3,3,3,5,0},
        {0,1,6,2,0,1,2,0,0,0,1,2,0,0,0,0,0,0,0,0,1,2,0,0,1,6,6,6,6,6,2,0},
        {0,1,0,2,0,1,2,0,4,3,6,6,3,5,0,4,3,3,3,3,6,6,3,3,6,0,0,0,0,0,2,0},
        {0,0,0,0,0,1,2,0,0,0,1,6,6,6,3,6,6,6,6,6,2,0,0,0,0,0,4,3,5,0,2,0},
        {0,4,3,3,3,6,6,3,5,0,1,2,0,1,2,0,0,0,0,1,2,0,4,0,4,3,2,0,1,0,2,0},
        {0,1,0,6,0,0,0,6,2,0,1,2,0,1,2,0,4,3,3,6,2,0,1,3,6,6,2,0,1,3,2,0},
        {0,1,0,1,3,5,0,6,2,0,0,0,0,1,2,0,0,0,1,6,6,3,6,2,0,0,0,0,1,6,2,0},
        {0,1,0,1,6,2,0,6,6,3,3,3,3,6,2,0,4,3,2,0,1,6,2,0,0,4,5,0,0,0,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,6,2,0,1,6,6,3,3,6,2,0,4,3,5,0},
        {0,4,3,6,5,0,4,3,3,5,0,4,3,3,3,3,6,6,2,0,1,0,0,0,0,6,2,0,1,6,2,0},
        {0,0,6,6,2,0,0,6,2,0,0,0,1,0,0,0,6,6,2,0,1,0,4,3,3,6,2,0,0,1,2,0},
        {0,4,6,6,6,3,3,6,2,0,4,3,6,3,5,0,0,0,0,0,1,0,0,0,1,6,2,0,1,6,2,0},
        {0,0,0,0,6,2,0,0,0,0,1,6,6,6,2,0,4,3,3,3,6,3,5,0,1,6,2,0,1,6,2,0},
        {0,4,0,4,6,6,3,3,5,0,1,0,0,0,2,0,1,2,0,1,6,6,2,0,1,6,6,3,2,0,2,0},
        {0,1,0,1,6,6,6,6,2,0,1,3,3,3,2,0,1,2,0,0,1,6,2,0,1,6,6,6,2,0,0,0},
        {0,1,0,1,6,2,0,1,6,3,6,6,1,0,0,0,1,0,0,4,6,6,2,0,0,0,0,0,1,3,5,0},
        {0,1,3,6,6,2,0,0,0,0,0,0,0,0,5,0,1,6,6,2,0,1,6,3,5,0,3,6,6,6,2,0},
        {0,1,6,6,6,2,0,4,3,3,5,0,4,3,2,0,1,6,6,2,0,0,1,2,0,0,6,6,6,0,2,0},
        {0,1,0,0,0,2,0,1,6,6,2,0,1,6,2,0,1,6,6,6,5,0,1,6,5,0,2,0,0,0,0,0},
        {0,1,3,5,0,2,0,0,0,1,2,0,1,0,0,0,0,0,6,0,1,6,6,6,2,0,2,0,1,0,5,0},
        {0,1,2,0,0,1,5,0,4,2,0,0,1,0,4,3,5,0,0,0,1,2,0,0,0,0,2,0,1,0,2,0},
        {0,1,6,5,0,1,6,3,6,6,3,3,6,3,6,6,6,3,3,3,6,6,3,3,3,3,6,6,6,6,2,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// Matriz para armazenar as variações dos tiles
    int tile_variations[TILE_ROWS][TILE_COLS];

// Bitmaps para os tiles
    ALLEGRO_BITMAP *wall_sprite; // Parede horizontal (80x80)
    ALLEGRO_BITMAP *left_shadow_floor_sprite; // Chão com sombra à esquerda (40x120, 3 variações)
    ALLEGRO_BITMAP *right_shadow_floor_sprite; // Chão com sombra à direita (40x120, 3 variações)
    ALLEGRO_BITMAP *top_shadow_floor_sprite; // Chão com sombra superior (40x80, 2 variações)
    ALLEGRO_BITMAP *top_left_shadow_floor_sprite; // Chão canto superior esquerdo (40x40)
    ALLEGRO_BITMAP *top_right_shadow_floor_sprite; // Chão canto superior direito (40x40)
    ALLEGRO_BITMAP *floor_sprite; // Chão sem sombra (80x120, 6 variações)

// Variável global para a máscara de luz
    ALLEGRO_BITMAP *light_mask = NULL;
    ALLEGRO_BITMAP *light_buffer = NULL;

// verificar se não é paredes
    bool can_move(float x, float y)
    {
        int tile_x = (int)((x + SPRITE_SIZE / 2) / TILE_SIZE);
        int tile_y = (int)((y + SPRITE_SIZE / 2) / TILE_SIZE);
        if (tile_x < 0 || tile_x >= TILE_COLS || tile_y < 0 || tile_y >= TILE_ROWS)
            return false;
        return map[tile_y][tile_x] != 0; // Apenas 0 é paredes agora
    }

// Função pra ver se tem parede entre dois pontos
    bool has_line_of_sight(float x1, float y1, float x2, float y2)
    {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance == 0) {
            return true; // Os pontos são os mesmos, linha de visão clara.
        }

    // normaliza o vetor de direção
        float step_x = dx / distance;
        float step_y = dy / distance;

        // define um tamanho de passo para a verificação.
        // valor menor = mais preciso
        float step_size = TILE_SIZE / 2.0f;

        // "Caminha" ao longo da linha do ponto 1 para o ponto 2
        for (float i = 0; i < distance; i += step_size) {
            float current_x = x1 + i * step_x;
            float current_y = y1 + i * step_y;

        // pega as coordenadas do tile no ponto atual da linha
            int tile_x = (int)(current_x / TILE_SIZE);
            int tile_y = (int)(current_y / TILE_SIZE);

        // verifica se as coordenadas estão dentro do mapa
            if (tile_x < 0 || tile_x >= TILE_COLS || tile_y < 0 || tile_y >= TILE_ROWS) {
                return false; // Fora do mapa é considerado bloqueado
            }

        // se o tile atual é uma parede (valor 0), a linha de visão está bloqueada.
            if (map[tile_y][tile_x] == 0) {
                return false;
            }
        }

    // se loop terminar = nenhuma parede encontrada no caminho.
    return true;
    }

// função para encontrar uma posição de spawn válida
typedef struct {
    int row, col;
} TilePosition;

    void find_valid_spawn_position(float *x, float *y, float player_x, float player_y, bool is_clone)
    {
        static TilePosition valid_tiles[TILE_ROWS * TILE_COLS];
        static int valid_tile_count = 0;
        static bool initialized = false;

        // inicializar lista de tiles válidos
        if (!initialized)
        {
            for (int row = 0; row < TILE_ROWS; row++) {
                for (int col = 0; col < TILE_COLS; col++) {
                    if (map[row][col] != 0) {
                        valid_tiles[valid_tile_count].row = row;
                        valid_tiles[valid_tile_count].col = col;
                        valid_tile_count++;
                    }
                }
            }
            initialized = true;
        }
        if (valid_tile_count == 0) {
            fprintf(stderr, "Erro: Nenhum tile válido para spawn.\n");
            *x = player_x;
            *y = player_y;
            return;
        }
        if (is_clone)
        {
            // Para o clone, spawn a uma distância específica
            int attempts = 0;
            const int max_attempts = 50;
            while (attempts < max_attempts)
            {
                float angle = (float)(rand() % 360) * M_PI / 180.0;
                float distance = MIN_NPC_DISTANCE + (float)(rand() % (MAX_NPC_DISTANCE - MIN_NPC_DISTANCE));
                float temp_x = player_x + distance * cos(angle);
                float temp_y = player_y + distance * sin(angle);
                if (can_move(temp_x, temp_y)) {
                    *x = temp_x;
                    *y = temp_y;
                    return;
                }
                attempts++;
            }
        }

        // escolher um tile válido aleatoriamente
        int index = rand() % valid_tile_count;
        *x = valid_tiles[index].col * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        *y = valid_tiles[index].row * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
    }

    // máscara de luz
    void create_light_mask()
    {
        int radius = LIGHT_RADIUS;
        light_mask = al_create_bitmap(radius * 2, radius * 2);
        if (!light_mask) {
            fprintf(stderr, "Erro ao criar máscara de luz.\n");
        return;
        }

        ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();
        al_set_target_bitmap(light_mask);
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
        for (int i = 0; i < radius; i++) {
            float alpha = (1.0f - (float)i / radius);
            al_draw_filled_circle(radius, radius, radius - i, al_map_rgba_f(1, 1, 1, alpha * alpha));
        }
    al_set_target_bitmap(prev_target);
    }



    void reset_game_state() // Função pra nova partida
    {
        player.sprite_sheet = character_sprites[rand() % NUM_NPCS];

        find_valid_spawn_position(&player.x, &player.y, 0, 0, false);
        player.frame = 0;
        player.movement = 0;
        player.alive = true;
        player.is_target = false;
        player.move_timer = 0;
        player.move_duration = 0;
        player.move_mode = 0;

        ALLEGRO_BITMAP *available_sprites[NUM_NPCS - 1];
        int available_count = 0;
        for (int j = 0; j < NUM_NPCS; j++) {
            if (character_sprites[j] != player.sprite_sheet) {
                available_sprites[available_count++] = character_sprites[j];
            }
        }

        for (int i = 0; i < NUM_NPCS; i++) {
            float spawn_x, spawn_y;
            find_valid_spawn_position(&spawn_x, &spawn_y, player.x, player.y, i == 0);
            npcs[i].x = spawn_x;
            npcs[i].y = spawn_y;
            npcs[i].sprite_sheet = (i == 0) ? player.sprite_sheet : available_sprites[i - 1];
            npcs[i].frame = 0;
            npcs[i].movement = 0;
            npcs[i].alive = true;
            npcs[i].is_target = (i == 0);
            npcs[i].move_timer = 0;
            npcs[i].move_duration = (rand() % 40 + 40) / 10.0;
            npcs[i].move_mode = rand() % 4;
            npcs[i].death_timer = 0.0f;
        }

        knife.active = false;
        mirror_item.active = true;
        find_valid_spawn_position(&mirror_item.x, &mirror_item.y, 0, 0, false);
        found_mirror = false;

        // Reseta as flags de controle do jogo
        game_over = false;
        victory = false;
    }


    void perform_fade(float duration, int direction) // fade na transição
    {
        if (duration <= 0) return;

        int num_steps = 60 * duration; // 60 FPS * duração em segundos
        float alpha_step = 255.0f / num_steps;

        for (int i = 0; i <= num_steps; i++) {
            float alpha = (direction == 1) ? (i * alpha_step) : (255 - (i * alpha_step));
            if (alpha < 0) alpha = 0;
            if (alpha > 255) alpha = 255;

            al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba(255, 255, 255, alpha));
            al_flip_display();
            al_rest(duration / num_steps);
        }
    }



//  TELA INICIAL
GameState show_main_menu(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *titulo_grande, ALLEGRO_FONT *btn_font, void (*fade_func)(float, int))
{
    int selected_option = 0; // 0: jogar, 1: sair
    const int total_options = 3; // jogar, instruções e sair

    float button_w = 300, button_h = 50;
    float button_spacing = 15;

    // posições de todos os botões
    float start_y = HEIGHT / 2 - button_h;
    float play_x = WIDTH / 2 - button_w / 2;
    float play_y = start_y;
    float instructions_x = play_x;
    float instructions_y = play_y + button_h + button_spacing;
    float exit_x = play_x;
    float exit_y = instructions_y + button_h + button_spacing;

    while (true)
    {
        ALLEGRO_EVENT ev; // eventos possiveis
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { // clicar no x
            return STATE_EXITING;

        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) // segurar tecla
        {

            if (ev.keyboard.keycode == ALLEGRO_KEY_W || ev.keyboard.keycode == ALLEGRO_KEY_UP) // mover
            {
                selected_option = (selected_option - 1 + total_options) % total_options;
            }
            if (ev.keyboard.keycode == ALLEGRO_KEY_S || ev.keyboard.keycode == ALLEGRO_KEY_DOWN) //
            {
                selected_option = (selected_option + 1) % total_options;
            }
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_SPACE) // apertar space
            {
                if (selected_option != 1) fade_func(0.7, 1); // fade-out, exceto para instruções
                if (selected_option == 0) return STATE_GAMEPLAY;     // jogar
                if (selected_option == 1) return STATE_INSTRUCTIONS; // instruções
                if (selected_option == 2) return STATE_EXITING;      // sair
            }

        }

       // Conteudos na tela
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(titulo_grande, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 4, ALLEGRO_ALIGN_CENTER, "Parece Comigo");

        // Botão Jogar
        al_draw_filled_rectangle(play_x, play_y, play_x + button_w, play_y + button_h, al_map_rgb(50, 50, 50));
        al_draw_text(btn_font, al_map_rgb(255, 255, 255), WIDTH / 2, play_y + 12, ALLEGRO_ALIGN_CENTER, "Jogar");

        // Botão Instruções
        al_draw_filled_rectangle(instructions_x, instructions_y, instructions_x + button_w, instructions_y + button_h, al_map_rgb(50, 50, 50));
        al_draw_text(btn_font, al_map_rgb(255, 255, 255), WIDTH / 2, instructions_y + 12, ALLEGRO_ALIGN_CENTER, "Instrucoes");

        // Botão Sair
        al_draw_filled_rectangle(exit_x, exit_y, exit_x + button_w, exit_y + button_h, al_map_rgb(50, 50, 50));
        al_draw_text(btn_font, al_map_rgb(255, 255, 255), WIDTH / 2, exit_y + 12, ALLEGRO_ALIGN_CENTER, "Sair do Jogo");

        // Lógica de seleção
        if (selected_option == 0) {
            al_draw_rectangle(play_x - 3, play_y - 3, play_x + button_w + 3, play_y + button_h + 3, al_map_rgb(255, 255, 255), 3);
        } else if (selected_option == 1) {
            al_draw_rectangle(instructions_x - 3, instructions_y - 3, instructions_x + button_w + 3, instructions_y + button_h + 3, al_map_rgb(255, 255, 255), 3);
        } else {
            al_draw_rectangle(exit_x - 3, exit_y - 3, exit_x + button_w + 3, exit_y + button_h + 3, al_map_rgb(255, 255, 255), 3);
        }

        al_flip_display();
    }
}


// TELA FINAL
GameState show_end_screen(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *titulo, ALLEGRO_FONT *sub, ALLEGRO_FONT *btn_font, const char* title_text, ALLEGRO_COLOR title_color, const char* subtitle_text, void (*fade_func)(float, int))
{
    bool done = false;
    bool restart_game = false;
    int selected_option = 0;

    float button_w = 250, button_h = 50;
    float respawn_x = WIDTH / 2 - button_w / 2, respawn_y = HEIGHT / 2 + 40;
    float menu_x = WIDTH / 2 - button_w / 2, menu_y = respawn_y + button_h + 15;

    while (!done)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
            restart_game = false;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (ev.keyboard.keycode == ALLEGRO_KEY_W || ev.keyboard.keycode == ALLEGRO_KEY_S ||
                ev.keyboard.keycode == ALLEGRO_KEY_A || ev.keyboard.keycode == ALLEGRO_KEY_D)
            {
                selected_option = 1 - selected_option;
            }

            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_SPACE)
            {
                fade_func(0.7, 1);
                done = true;
                restart_game = (selected_option == 0);
            }
        }

        // Conteudos na tela
        al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba_f(0, 0, 0, 0.6));
        al_draw_text(titulo, al_map_rgb(0, 0, 0), WIDTH / 2 + 4, HEIGHT / 4 + 4, ALLEGRO_ALIGN_CENTER, title_text);
        al_draw_text(titulo, title_color, WIDTH / 2, HEIGHT / 4, ALLEGRO_ALIGN_CENTER, title_text);
        al_draw_text(sub, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 4 + 70, ALLEGRO_ALIGN_CENTER, subtitle_text);

        // opção de novo jogo
        al_draw_filled_rectangle(respawn_x, respawn_y, respawn_x + button_w, respawn_y + button_h, al_map_rgb(50, 50, 50));
        al_draw_rectangle(respawn_x, respawn_y, respawn_x + button_w, respawn_y + button_h, al_map_rgb(0,0,0), 2);
        al_draw_text(btn_font, al_map_rgb(255, 255, 255), WIDTH / 2, respawn_y + 12, ALLEGRO_ALIGN_CENTER, "Nova partida");

        // opção de menu principal
        al_draw_filled_rectangle(menu_x, menu_y, menu_x + button_w, menu_y + button_h, al_map_rgb(50, 50, 50));
        al_draw_rectangle(menu_x, menu_y, menu_x + button_w, menu_y + button_h, al_map_rgb(0,0,0), 2);
        al_draw_text(btn_font, al_map_rgb(255, 255, 255), WIDTH / 2, menu_y + 12, ALLEGRO_ALIGN_CENTER, "Menu principal");

        if (selected_option == 0)
        {
            al_draw_rectangle(respawn_x - 3, respawn_y - 3, respawn_x + button_w + 3, respawn_y + button_h + 3, al_map_rgb(255, 255, 255), 3);
        } else
        {
            al_draw_rectangle(menu_x - 3, menu_y - 3, menu_x + button_w + 3, menu_y + button_h + 3, al_map_rgb(255, 255, 255), 3);

        }al_flip_display();
    }

    // lógica de retorno a outra tela
    if (restart_game)
    {
        return STATE_GAMEPLAY;
    } else
    {
        return STATE_MAIN_MENU;
    }
}



// TELA DE INSTRUÇÕES
GameState show_instructions_screen(ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *titulo_grande, ALLEGRO_FONT *texto_normal, ALLEGRO_FONT *btn_font)
{
    // Textos divididos em linhas separadas
    const char *intro_l1 = "Voce acorda em um lugar estranho e escuro,";
    const char *intro_l2 = "sem se lembrar de como chegou aqui.";
    const char *intro_l3 = "Seu rosto esta coberto, e um sentimento";
    const char *intro_l4 = "de pavor toma conta de voce.";

    const char *objetivo_titulo = "Objetivo:";
    const char *objetivo_l1 = "1. Encontre o ESPELHO para se lembrar.";
    const char *objetivo_l2 = "2. Apos se ver no espelho, voce descobrira que";
    const char *objetivo_l3 = "um CLONE, identico a voce, esta a espreita.";
    const char *objetivo_l4 = "3. Seu clone ira caca-lo. Voce precisa elimina-lo";
    const char *objetivo_l5 = "antes que ele o encontre.";

    // Posições
    float button_w = 200, button_h = 45;
    float button_x = WIDTH / 2 - button_w / 2;
    float button_y = HEIGHT - button_h - 25;

    while(true)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            return STATE_EXITING;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE || ev.keyboard.keycode == ALLEGRO_KEY_ENTER || ev.keyboard.keycode == ALLEGRO_KEY_SPACE)
            {
                return STATE_MAIN_MENU;
            }
        }

        al_clear_to_color(al_map_rgb(0, 0, 0));

        // Conteudos na tela
        float y_pos = 30;
        float line_height = al_get_font_line_height(texto_normal);
        float margin_left = 30; // Margem esquerda para o texto

        // 1. Título
        al_draw_text(titulo_grande, al_map_rgb(255, 255, 255), WIDTH / 2, y_pos, ALLEGRO_ALIGN_CENTER, "Instrucoes");
        y_pos += al_get_font_line_height(titulo_grande) + 25;

        // 2. Texto de introdução
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, intro_l1); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, intro_l2); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, intro_l3); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, intro_l4); y_pos += line_height;
        y_pos += 20; // Espaço extra após o parágrafo

        // 3. Título "Objetivo"
        al_draw_text(texto_normal, al_map_rgb(255, 255, 255), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, objetivo_titulo);
        y_pos += line_height + 5;

        // 4. Lista de objetivos
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, objetivo_l1); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, objetivo_l2); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, objetivo_l3); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, objetivo_l4); y_pos += line_height;
        al_draw_text(texto_normal, al_map_rgb(220, 220, 220), margin_left, y_pos, ALLEGRO_ALIGN_LEFT, objetivo_l5); y_pos += line_height;

        // 5. Botão de voltar
        al_draw_filled_rectangle(button_x, button_y, button_x + button_w, button_y + button_h, al_map_rgb(50, 50, 50));
        al_draw_rectangle(button_x - 2, button_y - 2, button_x + button_w + 2, button_y + button_h + 2, al_map_rgb(255, 255, 255), 2);
        al_draw_text(btn_font, al_map_rgb(255, 255, 255), WIDTH / 2, button_y + 10, ALLEGRO_ALIGN_CENTER, "Voltar");

        al_flip_display();
    }
}



// TELA DO JOGO
GameState run_gameplay_loop(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_TIMER* timer, ALLEGRO_FONT* font_titulo_grande, ALLEGRO_FONT* font_subtitulo, ALLEGRO_FONT* font_botao, ALLEGRO_BITMAP* censure_sprite, ALLEGRO_BITMAP* knife_frame, ALLEGRO_BITMAP* death_sprite_frame, ALLEGRO_SAMPLE_INSTANCE* hit_instance, ALLEGRO_SAMPLE_INSTANCE* current_gameplay_music_instance, ALLEGRO_SAMPLE_INSTANCE* item_collect_instance, ALLEGRO_SAMPLE_INSTANCE* voice_instances[], void (*fade_func)(float, int))
{
    bool gameplay_running = true;
    bool redraw = true;
    bool keys[4] = {false, false, false, false}; // W, S, A, D
    float camera_x = 0, camera_y = 0;
    double sprite_timer = 0.0, sprite_delay = 0.15;

    // conjuntos de diálogos possíveis
    const char* all_dialogue_sets[NUM_DIALOGUE_SETS][PHRASES_PER_SET] = {
    // choque
    {"Um espelho", "Espere...", "Essa pessoa...", "O rosto...", "E identico", "Entao... aquela pessoa", "Quem... e o real clone?", "ah nao"},
    // negação
    {"Que piada e essa?!", "Aquela coisa...", "E um truque", "Uma ilusao barata", "Eu nao sou isso...", "Aquele parecido comigo", "...", "So pode existir um"},
    // desespero
    {"Por favor", "Isso tem que parar", "O que eu fiz pra merecer isso?", "Ficar preso com um impostor", "Nao, nao, nao...", "Nao quero machucar ninguem,", "Preciso escapar daquilo", "..."},
    // confusão
    {"Quem sou eu?", "E quem e... ele?", "Se ele sou eu...", "Quem e o original?", "Nossos rostos...", "Sao os mesmos", "Vou acabar com isso", "So um e o real"},
    // reflexão
    {"Ah...", "Entao eu sou assim", "Essa sombra... me segue", "Sempre esteve aqui", "Eu so nao queria ver", "Nao tem mais pra onde fugir", "Preciso acabar...", "...Com isso"}
    };

    // Ponteiro para o conjunto de dialogos escolhido p partida
    const char** current_dialogue_set = NULL;

    // variaveis de controle
    bool is_in_dialogue = false;
    int dialogue_index = 0;
    int chars_to_draw = 0;
    float dialogue_char_timer = 0.0f;
    int chosen_voice_index = -1;

    // constantes do diálogo
    const float CHAR_TYPE_SPEED = 0.05f;
    const float PAUSE_BETWEEN_PHRASES = 1.0f;

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    // fade-in
    fade_func(0.7, -1);

    // música do jogo (depois da transição)
    if (current_gameplay_music_instance) al_play_sample_instance(current_gameplay_music_instance);


    while (gameplay_running)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        // Eventos que devem funcionar sempre
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            gameplay_running = false;
        }

        // Eventos que so funcionam durante o jogo
        if (!game_over) {
            switch (ev.type)
            {
                case ALLEGRO_EVENT_KEY_DOWN: // quando teclas são pressionadas
                    if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) gameplay_running = false;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_W) keys[0] = true;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_S) keys[1] = true;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_A) keys[2] = true;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_D) keys[3] = true;

                    if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE && !knife.active) // quando a barra de espaço é precionada
                    {
                        if (hit_instance) al_play_sample_instance(hit_instance);
                        knife.active = true;
                        knife.frame = 0;
                        knife.anim_timer = 0;

                        switch (player.movement) { // direções da faca
                            case 0: knife.x = player.x + (SPRITE_SIZE - KNIFE_WIDTH) / 2; knife.y = player.y + SPRITE_SIZE; break;
                            case 1: knife.x = player.x - KNIFE_WIDTH; knife.y = player.y + (SPRITE_SIZE - KNIFE_HEIGHT) / 2; break;
                            case 2: knife.x = player.x + SPRITE_SIZE; knife.y = player.y + (SPRITE_SIZE - KNIFE_HEIGHT) / 2; break;
                            case 3: knife.x = player.x + (SPRITE_SIZE - KNIFE_WIDTH) / 2; knife.y = player.y - KNIFE_HEIGHT; break;
                        }

                        // verificar se tem uma parede entre o jogador e o npc
                        for (int i = 0; i < NUM_NPCS; i++) {
                            if (npcs[i].alive)
                            {
                                float npc_center_x = npcs[i].x + SPRITE_SIZE / 2;
                                float npc_center_y = npcs[i].y + SPRITE_SIZE / 2;
                                float player_center_x = player.x + SPRITE_SIZE / 2;
                                float player_center_y = player.y + SPRITE_SIZE / 2;
                                float dx = npc_center_x - (knife.x + KNIFE_WIDTH / 2);
                                float dy = npc_center_y - (knife.y + KNIFE_HEIGHT / 2);

                                if (dx * dx + dy * dy < KNIFE_RANGE * KNIFE_RANGE && has_line_of_sight(player_center_x, player_center_y, npc_center_x, npc_center_y))
                                {
                                    npcs[i].alive = false;
                                    npcs[i].death_timer = 1.0f;

                                    if (npcs[i].is_target)
                                    {
                                        game_over = true;
                                        victory = true;
                                    } else {
                                        printf("NPC %d eliminado.\n", i);
                                    }
                                }
                            }
                        }
                    }
                    break;

                case ALLEGRO_EVENT_KEY_UP: // quando teclas são soltas
                    if (ev.keyboard.keycode == ALLEGRO_KEY_W) keys[0] = false;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_S) keys[1] = false;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_A) keys[2] = false;
                    if (ev.keyboard.keycode == ALLEGRO_KEY_D) keys[3] = false;
                    break;

            }
        }

        // atualização a cada frame
        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            if (!game_over)
            {
                // Animação do sprite do jogador e NPCs
                sprite_timer += 1.0 / 60.0;
                if (sprite_timer >= sprite_delay)
                {
                    if (keys[0] || keys[1] || keys[2] || keys[3]) {
                     player.frame = (player.frame + 1) % SPRITE_COLS;
                    } else {
                    player.frame = 0; // Personagem parado
                    }

                    for (int i = 0; i < NUM_NPCS; i++)
                    {
                        if (npcs[i].alive)
                        npcs[i].frame = (npcs[i].frame + 1) % SPRITE_COLS;
                    }
                sprite_timer = 0.0;
                }

                // Animação da faca
                if (knife.active)
                {
                    knife.anim_timer += 1.0 / 60.0;
                    if (knife.anim_timer >= KNIFE_ANIM_SPEED)
                    {
                        knife.frame++;
                        knife.anim_timer = 0;
                        if (knife.frame >= KNIFE_FRAMES)
                        {
                            knife.active = false;
                            knife.frame = 0;
                        }
                    }
                }

                // Movimento do jogador
                float speed = 3.0;
                float new_x = player.x, new_y = player.y;
                if (keys[0]) { new_y -= speed; player.movement = 3; }
                if (keys[1]) { new_y += speed; player.movement = 0; }
                if (keys[2]) { new_x -= speed; player.movement = 1; }
                if (keys[3]) { new_x += speed; player.movement = 2; }
                if (can_move(new_x, new_y)) { player.x = new_x; player.y = new_y; }

                // Limites do mapa para o jogador
                if (player.x < 0) player.x = 0;
                if (player.x > MAP_WIDTH - SPRITE_SIZE) player.x = MAP_WIDTH - SPRITE_SIZE;
                if (player.y < 0) player.y = 0;
                if (player.y > MAP_HEIGHT - SPRITE_SIZE) player.y = MAP_HEIGHT - SPRITE_SIZE;

                // Lógica da câmera
                camera_x = player.x - WIDTH / 2;
                camera_y = player.y - HEIGHT / 2;
                if (camera_x < 0) camera_x = 0;
                if (camera_x > MAP_WIDTH - WIDTH) camera_x = MAP_WIDTH - WIDTH;
                if (camera_y < 0) camera_y = 0;
                if (camera_y > MAP_HEIGHT - HEIGHT) camera_y = MAP_HEIGHT - HEIGHT;

                // Lógica do espelho
                if (mirror_item.active)
                {
                    float dx_mirror = player.x - mirror_item.x;
                    float dy_mirror = player.y - mirror_item.y;
                    if (sqrt(dx_mirror * dx_mirror + dy_mirror * dy_mirror) < (SPRITE_SIZE / 2))
                    {
                        if (item_collect_instance) al_play_sample_instance(item_collect_instance);
                        found_mirror = true;
                        mirror_item.active = false;
                        printf("Voce encontrou o espelho!\n");

                        // parte do dialogo logo apos pegar espelho
                        is_in_dialogue = true;
                        chosen_voice_index = rand() % NUM_VOICE_SOUNDS;

                        int set_index = rand() % NUM_DIALOGUE_SETS;
                        current_dialogue_set = all_dialogue_sets[set_index];

                        if (chosen_voice_index != -1 && voice_instances[chosen_voice_index]) {
                            al_play_sample_instance(voice_instances[chosen_voice_index]);
                        }
                    }
                }

                // Lógica dos NPCs
                for (int i = 0; i < NUM_NPCS; i++)
                {
                    if (npcs[i].alive)
                    {
                        float new_x_npc = npcs[i].x;
                        float new_y_npc = npcs[i].y;
                        float npc_speed = npcs[i].is_target ? 2.0 : 1.5;
                        bool is_pursuing = npcs[i].is_target;

                        if (!npcs[i].is_target)
                        {
                            npcs[i].move_timer -= 1.0 / 60.0;
                            if (npcs[i].move_timer <= 0)
                            {
                                npcs[i].move_timer = (rand() % 60 + 40) / 10.0;
                                if ((rand() % 100) < CHANCE_TO_PURSUE) npcs[i].move_mode = -1; // chance de perseguir
                                else npcs[i].move_mode = rand() % 4; // movimento randomico
                            }
                            if (npcs[i].move_mode == -1) is_pursuing = true;
                        }

                        // logica de colisão
                        if (is_pursuing)
                        {
                            float dx = player.x - npcs[i].x;
                            float dy = player.y - npcs[i].y;
                            float dist = sqrt(dx * dx + dy * dy);

                            if (dist > 1)
                            {
                                float move_x = (dx / dist) * npc_speed;
                                float move_y = (dy / dist) * npc_speed;
                                if (can_move(npcs[i].x + move_x, npcs[i].y + move_y))
                                {
                                    new_x_npc += move_x; new_y_npc += move_y; // percurso ideal
                                } else  // se nao conseguiu
                                {
                                    if (can_move(npcs[i].x + move_x, npcs[i].y)) new_x_npc += move_x; // mover em x
                                    else if (can_move(npcs[i].x, npcs[i].y + move_y)) new_y_npc += move_y; // mover em y
                                    else if (!npcs[i].is_target) npcs[i].move_timer = 0; // forca nova dicisão
                                }
                            }

                            float angle = atan2(dy, dx) * 180 / M_PI;
                            if (angle >= -45 && angle < 45) npcs[i].movement = 2;
                            else if (angle >= 45 && angle < 135) npcs[i].movement = 0;
                            else if (angle >= 135 || angle < -135) npcs[i].movement = 1;
                            else npcs[i].movement = 3;
                        } else
                        {
                            switch (npcs[i].move_mode) // maneiras como npc pode agir
                            {
                                case 0: new_y_npc -= npc_speed; npcs[i].movement = 3; break;
                                case 1: new_y_npc += npc_speed; npcs[i].movement = 0; break;
                                case 2: new_x_npc -= npc_speed; npcs[i].movement = 1; break;
                                case 3: new_x_npc += npc_speed; npcs[i].movement = 2; break;
                            }
                            if (!can_move(new_x_npc, new_y_npc))
                            {
                                npcs[i].move_timer = 0; new_x_npc = npcs[i].x; new_y_npc = npcs[i].y;
                            }
                        }

                        npcs[i].x = new_x_npc;
                        npcs[i].y = new_y_npc;

                        // Lógica de derrota (ser pego pelo clone)
                        if (npcs[i].is_target)
                        {
                            float final_dist = sqrt(pow(player.x - npcs[i].x, 2) + pow(player.y - npcs[i].y, 2));
                            if (final_dist < 30 && has_line_of_sight(player.x, player.y, npcs[i].x, npcs[i].y))
                            {
                                game_over = true;
                                victory = false;
                                if (chosen_voice_index != -1 && voice_instances[chosen_voice_index])
                                {
                                    al_stop_sample_instance(voice_instances[chosen_voice_index]);
                                }
                            }
                        }
                    } // fim do if(npcs[i].alive)

                    else if (npcs[i].death_timer > 0)
                    {
                        npcs[i].death_timer -= 1.0 / 60.0;
                    }
                }
            }


            // logica de dialogo
            if (is_in_dialogue)
            {
                dialogue_char_timer += 1.0 / 60.0;
                const char* full_phrase = current_dialogue_set[dialogue_index];
                // calcula o tamanho da frase atual
                int current_phrase_len = strlen(full_phrase);

                // logica p digitação do texto
                if (chars_to_draw < current_phrase_len)
                {
                    if (dialogue_char_timer >= CHAR_TYPE_SPEED)
                    {
                        dialogue_char_timer = 0;
                        chars_to_draw++;
                    }
                }
                else // a frase atual terminou de ser digitada
                {
                // parar som que tava tocando em loop
                    if (chosen_voice_index != -1 && voice_instances[chosen_voice_index]) {
                        al_stop_sample_instance(voice_instances[chosen_voice_index]);
                    }

                    // espera a pausa entre as frases
                    if (dialogue_char_timer >= PAUSE_BETWEEN_PHRASES)
                    {
                        // reseta o timer e avança para a proxima frase
                        dialogue_char_timer = 0;
                        dialogue_index++;
                        chars_to_draw = 0; // reseta os caracteres p nova frase

                        // ver se o dialogo inteiro acabou
                        if (dialogue_index >= PHRASES_PER_SET) //
                        {
                            is_in_dialogue = false; // termina o diálogo
                            chosen_voice_index = -1; // reseta a voz escolhida
                        }
                        else // se não acabou, prepara e inicia o som p proxima frase
                        {
                        // inicia o som da nova frase
                            if (chosen_voice_index != -1 && voice_instances[chosen_voice_index]) {
                                al_play_sample_instance(voice_instances[chosen_voice_index]);
                            }
                        }
                    }
                }
            }

        redraw = true; // redesenhar a tela
        }


        // desenhar tudo
        if (redraw && al_is_event_queue_empty(queue))
        {
            redraw = false;

            // desenhar toda a cena do jogo no buffer do tamanho do mapa (p iluminação)
            al_set_target_bitmap(light_buffer);
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // desenho do mapa matriz
            for (int row = 0; row < TILE_ROWS; row++)
            {
                for (int col = 0; col < TILE_COLS; col++)
                {
                    // As coordenadas de desenho agora são coordenadas do mapa
                    float draw_x = col * TILE_SIZE;
                    float draw_y = row * TILE_SIZE;
                    int tile_type = map[row][col];
                    int variation = tile_variations[row][col];

                    switch (tile_type) // desenhar paredes e tipos de chão
                    {
                        case 0: al_draw_bitmap(wall_sprite, draw_x, draw_y, 0); break;
                        case 1: al_draw_bitmap_region(left_shadow_floor_sprite, 0, variation * TILE_SIZE, TILE_SIZE, TILE_SIZE, draw_x, draw_y, 0); break;
                        case 2: al_draw_bitmap_region(right_shadow_floor_sprite, 0, variation * TILE_SIZE, TILE_SIZE, TILE_SIZE, draw_x, draw_y, 0); break;
                        case 3: al_draw_bitmap_region(top_shadow_floor_sprite, 0, variation * TILE_SIZE, TILE_SIZE, TILE_SIZE, draw_x, draw_y, 0); break;
                        case 4: al_draw_bitmap(top_left_shadow_floor_sprite, draw_x, draw_y, 0); break;
                        case 5: al_draw_bitmap(top_right_shadow_floor_sprite, draw_x, draw_y, 0); break;
                        case 6: al_draw_bitmap_region(floor_sprite, (variation % 2) * TILE_SIZE, (variation / 2) * TILE_SIZE, TILE_SIZE, TILE_SIZE, draw_x, draw_y, 0); break;
                    }
                }
            }

            // Desenhar espelho
            if (mirror_item.active)
            {
                al_draw_bitmap(mirror_item.sprite, mirror_item.x, mirror_item.y, 0);
            }

            // Desenhar jogador
            if (player.alive && player.sprite_sheet)
            {
                int sprite_x = player.frame * SPRITE_SIZE;
                int sprite_y = player.movement * SPRITE_SIZE;
                al_draw_bitmap_region(player.sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, player.x, player.y, 0);

                // desenhar censura
                if (!found_mirror && censure_sprite)
                {
                    al_draw_bitmap(censure_sprite, player.x, player.y, 0);
                }
            }

            // Desenhar faca
            if (knife.active && knife.sprite_sheet)
            {
                int sprite_x = knife.frame * KNIFE_WIDTH;
                int sprite_y = 0;
                float draw_x = knife.x;
                float draw_y = knife.y;
                al_set_target_bitmap(knife_frame);
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                al_draw_bitmap_region(knife.sprite_sheet, sprite_x, sprite_y, KNIFE_WIDTH, KNIFE_HEIGHT, 0, 0, 0);
                al_set_target_bitmap(light_buffer); // volta a desenhar no buffer do mapa

                switch (player.movement) // direções da faca
                {
                    case 0: al_draw_rotated_bitmap(knife_frame, KNIFE_WIDTH / 2, KNIFE_HEIGHT / 2, draw_x + KNIFE_WIDTH / 2, draw_y + KNIFE_HEIGHT / 2, M_PI / 2, 0); break;
                    case 1: al_draw_bitmap_region(knife.sprite_sheet, sprite_x, sprite_y, KNIFE_WIDTH, KNIFE_HEIGHT, draw_x, draw_y, ALLEGRO_FLIP_HORIZONTAL); break;
                    case 2: al_draw_bitmap_region(knife.sprite_sheet, sprite_x, sprite_y, KNIFE_WIDTH, KNIFE_HEIGHT, draw_x, draw_y, 0); break;
                    case 3: al_draw_rotated_bitmap(knife_frame, KNIFE_WIDTH / 2, KNIFE_HEIGHT / 2, draw_x + KNIFE_WIDTH / 2, draw_y + KNIFE_HEIGHT / 2, -M_PI / 2, 0); break;
                }
            }

            // Desenhar NPCs
            for (int i = 0; i < NUM_NPCS; i++)
            {
                if ((npcs[i].alive || npcs[i].death_timer > 0) && npcs[i].sprite_sheet)
                {
                    if (npcs[i].alive)
                    {
                        int sprite_x = npcs[i].frame * SPRITE_SIZE;
                        int sprite_y = npcs[i].movement * SPRITE_SIZE;
                        al_draw_bitmap_region(npcs[i].sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, npcs[i].x, npcs[i].y, 0);
                    } else
                    {
                        int sprite_x = 0;
                        int sprite_y = 0;
                        float draw_x = npcs[i].x;
                        float draw_y = npcs[i].y;

                        ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();
                        al_set_target_bitmap(death_sprite_frame);
                        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                        al_draw_bitmap_region(npcs[i].sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, 0, 0, 0);
                        al_set_target_bitmap(prev_target);
                        al_draw_tinted_rotated_bitmap(death_sprite_frame, al_map_rgb(255, 120, 120), SPRITE_SIZE / 2.0f, SPRITE_SIZE / 2.0f, draw_x + SPRITE_SIZE / 2.0f, draw_y + SPRITE_SIZE / 2.0f, M_PI / 2.0f, 0);
                    }
                }
            }

            // Desenhar o buffer na tela, colocando câmera e iluminação
            al_set_target_backbuffer(display);
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // Desenhar iluminção
            if (light_mask)
            {
                // 1. Desenha tudo escuro, pegando a região da câmera
                al_draw_tinted_bitmap_region
                (
                    light_buffer,
                    al_map_rgb_f(1.0 - AMBIENT_DARKNESS, 1.0 - AMBIENT_DARKNESS, 1.0 - AMBIENT_DARKNESS),
                    camera_x, camera_y,
                    WIDTH, HEIGHT,
                    0, 0,
                    0
                );

                // 2. Prepara para adicionar luz por cima
                al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);

                // 3. Desenha um pedaço do cenario ja iluminado por cima do escuro.

                al_draw_bitmap_region(
                    light_buffer,       // A origem é o buffer totalmente iluminado
                    (player.x + SPRITE_SIZE / 2) - LIGHT_RADIUS, (player.y + SPRITE_SIZE / 2) - LIGHT_RADIUS,       // Pega a região ao redor do centro do jogador NO MAPA
                    LIGHT_RADIUS * 2, LIGHT_RADIUS * 2,     // O tamanho da luz
                    (player.x - camera_x + SPRITE_SIZE / 2) - LIGHT_RADIUS, (player.y - camera_y + SPRITE_SIZE / 2) - LIGHT_RADIUS,     // Cola na posição correta NA TELA, centrado
                    0
                );

                // 4. Restaura o modo de desenho padrão
                al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

            }else
            {
                // se a luz falhar, apenas desenha a cena sem efeitos
                al_draw_bitmap_region(light_buffer, camera_x, camera_y, WIDTH, HEIGHT, 0, 0, 0);
            }


            // desenho do dialogo-
            if (is_in_dialogue)
            {
                float box_h = 80;
                // desenha a caixa
                al_draw_filled_rectangle(20, HEIGHT - box_h - 20, WIDTH - 20, HEIGHT - 20, al_map_rgba_f(0, 0, 0, 0.7));
                // desenha a borda
                al_draw_rectangle(20, HEIGHT - box_h - 20, WIDTH - 20, HEIGHT - 20, al_map_rgb(255, 255, 255), 2.0);

                // prepara e desenha o texto parcial
                if (chars_to_draw > 0)
                {
                    // Pega a frase completa
                    const char* full_phrase = current_dialogue_set[dialogue_index];

                    // Cria um buffer temporário para o texto parcial
                    char partial_text[256] = {0}; // Inicializa com zeros

                    // Copia apenas os caracteres necessários
                    int len_to_copy = chars_to_draw;
                    if (len_to_copy > strlen(full_phrase)) {
                        len_to_copy = strlen(full_phrase);
                    }
                    if (len_to_copy >= sizeof(partial_text)) {
                        len_to_copy = sizeof(partial_text) - 1;
                    }

                    // Copia manualmente para garantir o terminador nulo
                    for (int i = 0; i < len_to_copy; i++) {
                        partial_text[i] = full_phrase[i];
                    }

                    // Desenha o texto na caixa
                    al_draw_text(font_subtitulo, al_map_rgb(255, 255, 255), 40, HEIGHT - box_h, 0, partial_text);
                }
            }



            // Desenhar a tela de Fim de Jogo e virar o display
            if (game_over)
            {
                // Parar todos os sons ativos do gameplay
                if (current_gameplay_music_instance) al_stop_sample_instance(current_gameplay_music_instance);

                // para o som do diálogo
                if (chosen_voice_index != -1 && voice_instances[chosen_voice_index]) {
                    al_stop_sample_instance(voice_instances[chosen_voice_index]);
                }

                al_stop_timer(timer); // Para o timer do jogo

                GameState next_state;
                if (victory)
                {
                    next_state = show_end_screen(display, queue, font_titulo_grande, font_subtitulo, font_botao, "VITORIA!", al_map_rgb(0, 255, 0), "Voce matou o clone", fade_func);
                } else
                {
                    next_state = show_end_screen(display, queue, font_titulo_grande, font_subtitulo, font_botao, "Voce morreu!", al_map_rgb(255, 0, 0), "o clone te encontrou", fade_func);
                }
                return next_state; // Retorna para o loop principal do main
            }

            al_flip_display();

        } // Fim do if(redraw)
    } // Fim do while

    return STATE_EXITING; // caso ESC

} // fim da tela de jogo




int main()
{
// Inicializar
    if (!al_init()) {
        fprintf(stderr, "Erro ao inicializar o Allegro.\n");
        return -1;
    }
    if (!al_init_font_addon() || !al_init_ttf_addon()) {
        fprintf(stderr, "Erro ao inicializar addons de fonte.\n");
        return -1;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Erro ao inicializar addon de primitivas.\n");
        return -1;
    }
    if (!al_init_image_addon()) {
        fprintf(stderr, "Erro ao inicializar addon de imagem.\n");
        return -1;
    }
    if (!al_install_mouse()) {
        fprintf(stderr, "Erro ao inicializar o mouse.\n");
        return -1;
    }
    if (!al_install_keyboard()) {
        fprintf(stderr, "Erro ao inicializar o teclado.\n");
        return -1;
    }
    if (!al_install_audio() || !al_init_acodec_addon()) {
        fprintf(stderr, "Aviso: Erro ao inicializar áudio. Continuando sem som.\n");
    }

    ALLEGRO_DISPLAY *display = al_create_display(WIDTH, HEIGHT);
    if (!display) {
        fprintf(stderr, "Erro ao criar a janela.\n");
        return -1;
    }

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    if (!queue) {
        fprintf(stderr, "Erro ao criar fila de eventos.\n");
        al_destroy_display(display);
        return -1;
    }

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    if (!timer) {
        fprintf(stderr, "Erro ao criar temporizador.\n");
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }


    srand(time(NULL));

    // para a tela de Game Over
    ALLEGRO_FONT *font_titulo_grande = al_load_ttf_font("../../fontes/minecraft.ttf", 48, 0);
    ALLEGRO_FONT *font_subtitulo = al_load_ttf_font("../../fontes/minecraft.ttf", 24, 0);
    ALLEGRO_FONT *font_botao = al_load_ttf_font("../../fontes/minecraft.ttf", 22, 0);

    // luz
    light_buffer = al_create_bitmap(MAP_WIDTH, MAP_HEIGHT);
    create_light_mask(); // cria a máscara
    if (!light_buffer || !light_mask) {
        fprintf(stderr, "Erro ao criar recursos de iluminação.\n");
        return -1;
    }

    // Sons
    ALLEGRO_SAMPLE *hit_sample = NULL; // som da faca
    ALLEGRO_SAMPLE_INSTANCE *hit_instance = NULL;
    ALLEGRO_SAMPLE *gameplay_music_samples[NUM_GAMEPLAY_TRACKS] = {NULL}; // musicas da gameplay
    ALLEGRO_SAMPLE_INSTANCE *gameplay_music_instances[NUM_GAMEPLAY_TRACKS] = {NULL};
    ALLEGRO_SAMPLE *item_collect_sample = NULL; // som de coleta de item
    ALLEGRO_SAMPLE_INSTANCE *item_collect_instance = NULL;
    ALLEGRO_SAMPLE *menu_music = NULL; // musica do menu
    ALLEGRO_SAMPLE_INSTANCE *menu_instance = NULL;

    ALLEGRO_SAMPLE *voice_samples[NUM_VOICE_SOUNDS] = {NULL};
    ALLEGRO_SAMPLE_INSTANCE *voice_instances[NUM_VOICE_SOUNDS] = {NULL};


    if (al_install_audio())
    {
        al_reserve_samples(3); // Hit + música de fundo + dialogo do personagem

        hit_sample = al_load_sample("../../sons/hit.wav");
        if (!hit_sample)
        {
            fprintf(stderr, "Aviso: Não foi possível carregar som '../../sons/hit.wav'. Continuando sem som de hit.\n");
        } else
        {
            hit_instance = al_create_sample_instance(hit_sample);
            if (hit_instance) {
                al_attach_sample_instance_to_mixer(hit_instance, al_get_default_mixer());
            } else {
                fprintf(stderr, "Aviso: Não foi possível criar instância de som de hit.\n");
            }
        }


        // Carrega o som de coleta do item
        item_collect_sample = al_load_sample("../../sons/item_collect.wav");
        if (!item_collect_sample) {
            fprintf(stderr, "Aviso: Não foi possível carregar som '../../sons/item_collect.wav'.\n");
        } else {
            item_collect_instance = al_create_sample_instance(item_collect_sample);
            if (item_collect_instance) {
                al_attach_sample_instance_to_mixer(item_collect_instance, al_get_default_mixer());
            }
        }

        // Carrega as 8 músicas de gameplay em um loop
        for (int i = 0; i < NUM_GAMEPLAY_TRACKS; i++)
        {
            char filename[50];
            sprintf(filename, "../../sons/track_%d.wav", i + 1); // Cria o nome do arquivo: track_1.wav, track_2.wav, etc.

            gameplay_music_samples[i] = al_load_sample(filename);
            if (!gameplay_music_samples[i])
            {
                fprintf(stderr, "Aviso: Não foi possível carregar música '%s'.\n", filename);
            } else
            {
                gameplay_music_instances[i] = al_create_sample_instance(gameplay_music_samples[i]);
                if (gameplay_music_instances[i])
                {
                    al_set_sample_instance_playmode(gameplay_music_instances[i], ALLEGRO_PLAYMODE_LOOP);
                    al_attach_sample_instance_to_mixer(gameplay_music_instances[i], al_get_default_mixer());
                } else
                {
                    fprintf(stderr, "Aviso: Não foi possível criar instância para a música '%s'.\n", filename);
                }
            }
        }

        // Carrega os 7 sons de voz do diálogo
        for (int i = 0; i < NUM_VOICE_SOUNDS; i++) {
        char filename[50];
        sprintf(filename, "../../sons/voice%d.wav", i + 1);

        voice_samples[i] = al_load_sample(filename);
            if (!voice_samples[i]) {
                fprintf(stderr, "Aviso: Não foi possível carregar som '%s'.\n", filename);
            } else {
                voice_instances[i] = al_create_sample_instance(voice_samples[i]);
                if (voice_instances[i]) {
                    al_set_sample_instance_playmode(voice_instances[i], ALLEGRO_PLAYMODE_LOOP);
                    al_attach_sample_instance_to_mixer(voice_instances[i], al_get_default_mixer());
                }
            }
        }


        menu_music = al_load_sample("../../sons/omori.wav");
        if (!menu_music)
        {
            fprintf(stderr, "Aviso: Não foi possível carregar música '../../sons/omori.wav'.\n");
        } else
        {
            menu_instance = al_create_sample_instance(menu_music);
            if (menu_instance)
            {
                al_set_sample_instance_playmode(menu_instance, ALLEGRO_PLAYMODE_LOOP);
            al_attach_sample_instance_to_mixer(menu_instance, al_get_default_mixer());
            } else
            {
                fprintf(stderr, "Aviso: Não foi possível criar instância de música do menu.\n");
            }
        }

    }

    // personagens
    ALLEGRO_BITMAP *mia_sprite = al_load_bitmap("../../sprites/mia.png");
    if (!mia_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/mia.png'.\n");
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(mia_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *lucia_sprite = al_load_bitmap("../../sprites/lucia.png");
    if (!lucia_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/lucia.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(lucia_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *bruno_sprite = al_load_bitmap("../../sprites/bruno.png");
    if (!bruno_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/bruno.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(bruno_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *miguel_sprite = al_load_bitmap("../../sprites/miguel.png");
    if (!miguel_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/miguel.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(miguel_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *fernando_sprite = al_load_bitmap("../../sprites/fernando.png");
    if (!fernando_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/fernando.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(fernando_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *julia_sprite = al_load_bitmap("../../sprites/julia.png");
    if (!julia_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/julia.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(julia_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *lucas_sprite = al_load_bitmap("../../sprites/lucas.png");
    if (!lucas_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/lucas.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(lucas_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *monica_sprite = al_load_bitmap("../../sprites/monica.png");
    if (!monica_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/monica.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(monica_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *jay_sprite = al_load_bitmap("../../sprites/jay.png");
    if (!jay_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/jay.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(jay_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *amanda_sprite = al_load_bitmap("../../sprites/amanda.png");
    if (!amanda_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/amanda.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(amanda_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *marcos_sprite = al_load_bitmap("../../sprites/marcos.png");
    if (!marcos_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/marcos.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(marcos_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *nick_sprite = al_load_bitmap("../../sprites/nick.png");
    if (!nick_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/nick.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(nick_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *judite_sprite = al_load_bitmap("../../sprites/judite.png");
    if (!judite_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/judite.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(judite_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *daniel_sprite = al_load_bitmap("../../sprites/daniel.png");
    if (!daniel_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/daniel.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(daniel_sprite, al_map_rgb(255, 0, 255));


    ALLEGRO_BITMAP *fisky_sprite = al_load_bitmap("../../sprites/fisky.png");
    if (!fisky_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/fisky.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(fisky_sprite, al_map_rgb(255, 0, 255));


    // Sprite da faca
    ALLEGRO_BITMAP *knife_sprite = al_load_bitmap("../../sprites/knife.png");
    if (!knife_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/knife.png'.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(knife_sprite, al_map_rgb(255, 255, 255));

// Criar bitmap temporário para a faca
    ALLEGRO_BITMAP *knife_frame = al_create_bitmap(KNIFE_WIDTH, KNIFE_HEIGHT);
    if (!knife_frame) {
        fprintf(stderr, "Erro ao criar bitmap temporário para a faca.\n");
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }

    // animação de morte
    ALLEGRO_BITMAP *death_sprite_frame = al_create_bitmap(SPRITE_SIZE, SPRITE_SIZE);
    if (!death_sprite_frame) {
        fprintf(stderr, "Erro ao criar bitmap temporário para a morte.\n");
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }


    // carregar censura e espelho
    ALLEGRO_BITMAP *censure_sprite = al_load_bitmap("../../sprites/censure.png");
    if (!censure_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/censure.png'.\n");
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(censure_sprite, al_map_rgb(255, 0, 255));

    ALLEGRO_BITMAP *mirror_sprite = al_load_bitmap("../../sprites/mirror.png");
    if (!mirror_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/mirror.png'.\n");
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(mirror_sprite, al_map_rgb(0, 0, 0));


// Sprites do mapa
    wall_sprite = al_load_bitmap("../../map/wall.png");
    if (!wall_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/wall.png'.\n");
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(wall_sprite, al_map_rgb(255, 0, 255));

    left_shadow_floor_sprite = al_load_bitmap("../../map/left_shadow_floor.png");
    if (!left_shadow_floor_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/left_shadow_floor.png'.\n");
        al_destroy_bitmap(wall_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(left_shadow_floor_sprite, al_map_rgb(255, 0, 255));

    right_shadow_floor_sprite = al_load_bitmap("../../map/right_shadow_floor.png");
    if (!right_shadow_floor_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/right_shadow_floor.png'.\n");
        al_destroy_bitmap(left_shadow_floor_sprite);
        al_destroy_bitmap(wall_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(fisky_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(right_shadow_floor_sprite, al_map_rgb(255, 0, 255));

    top_shadow_floor_sprite = al_load_bitmap("../../map/top_shadow_floor.png");
    if (!top_shadow_floor_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/top_shadow_floor.png'.\n");
        al_destroy_bitmap(right_shadow_floor_sprite);
        al_destroy_bitmap(left_shadow_floor_sprite);
        al_destroy_bitmap(wall_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(fisky_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(top_shadow_floor_sprite, al_map_rgb(255, 0, 255));

    top_left_shadow_floor_sprite = al_load_bitmap("../../map/top_left_shadow_floor.png");
    if (!top_left_shadow_floor_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/top_left_shadow_floor.png'.\n");
        al_destroy_bitmap(top_shadow_floor_sprite);
        al_destroy_bitmap(right_shadow_floor_sprite);
        al_destroy_bitmap(left_shadow_floor_sprite);
        al_destroy_bitmap(wall_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(fisky_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(top_left_shadow_floor_sprite, al_map_rgb(255, 0, 255));

    top_right_shadow_floor_sprite = al_load_bitmap("../../map/top_right_shadow_floor.png");
    if (!top_right_shadow_floor_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/top_right_shadow_floor.png'.\n");
        al_destroy_bitmap(top_left_shadow_floor_sprite);
        al_destroy_bitmap(top_shadow_floor_sprite);
        al_destroy_bitmap(right_shadow_floor_sprite);
        al_destroy_bitmap(left_shadow_floor_sprite);
        al_destroy_bitmap(wall_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(fisky_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(top_right_shadow_floor_sprite, al_map_rgb(255, 0, 255));

    floor_sprite = al_load_bitmap("../../map/floor.png");
    if (!floor_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../map/floor.png'.\n");
        al_destroy_bitmap(top_right_shadow_floor_sprite);
        al_destroy_bitmap(top_left_shadow_floor_sprite);
        al_destroy_bitmap(top_shadow_floor_sprite);
        al_destroy_bitmap(right_shadow_floor_sprite);
        al_destroy_bitmap(left_shadow_floor_sprite);
        al_destroy_bitmap(wall_sprite);
        al_destroy_bitmap(knife_frame);
        al_destroy_bitmap(mia_sprite);
        al_destroy_bitmap(lucia_sprite);
        al_destroy_bitmap(bruno_sprite);
        al_destroy_bitmap(miguel_sprite);
        al_destroy_bitmap(fernando_sprite);
        al_destroy_bitmap(lucas_sprite);
        al_destroy_bitmap(julia_sprite);
        al_destroy_bitmap(monica_sprite);
        al_destroy_bitmap(jay_sprite);
        al_destroy_bitmap(amanda_sprite);
        al_destroy_bitmap(marcos_sprite);
        al_destroy_bitmap(nick_sprite);
        al_destroy_bitmap(judite_sprite);
        al_destroy_bitmap(daniel_sprite);
        al_destroy_bitmap(fisky_sprite);
        al_destroy_bitmap(knife_sprite);
        al_destroy_bitmap(death_sprite_frame);
        al_destroy_bitmap(censure_sprite);
        al_destroy_bitmap(mirror_sprite);
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample(gameplay_music_samples);
        al_destroy_sample_instance(gameplay_music_instances);
        al_destroy_sample(item_collect_sample);
        al_destroy_sample_instance(item_collect_instance);
        al_destroy_sample(voice_samples);
        al_destroy_sample_instance(voice_instances);
        al_destroy_sample(menu_music);
        al_destroy_sample_instance(menu_instance);
        al_destroy_font(font_titulo_grande);
        al_destroy_font(font_subtitulo);
        al_destroy_font(font_botao);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(floor_sprite, al_map_rgb(255, 0, 255));


    // Inicializar variações dos tiles
    srand(time(NULL));
    for (int row = 0; row < TILE_ROWS; row++)
    {
        for (int col = 0; col < TILE_COLS; col++)
        {
            switch (map[row][col])
            {
                case 1: tile_variations[row][col] = rand() % 3; break;
                case 2: tile_variations[row][col] = rand() % 3; break;
                case 3: tile_variations[row][col] = rand() % 2; break;
                case 6: tile_variations[row][col] = rand() % 6; break;
                default: tile_variations[row][col] = 0; break;
            }
        }
    }

    ALLEGRO_BITMAP* char_sprites_temp[] = // char com os personagens
    {
        mia_sprite, lucia_sprite, bruno_sprite, miguel_sprite, fernando_sprite,
        julia_sprite, lucas_sprite, monica_sprite, jay_sprite, amanda_sprite,
        marcos_sprite, nick_sprite, judite_sprite, daniel_sprite, fisky_sprite
    };

    for(int i = 0; i < NUM_NPCS; i++)
    {
        character_sprites[i] = char_sprites_temp[i];
    }
    // sprites que só precisam ser definidos uma vez
    knife.sprite_sheet = knife_sprite;
    mirror_item.sprite = mirror_sprite;

    // registro de eventos
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    GameState current_state = STATE_MAIN_MENU;
    bool running = true;

    while (running)
    {
        if (current_state == STATE_MAIN_MENU)
        {
            // Toca a música do menu
            if (menu_instance) al_play_sample_instance(menu_instance);
            // Chama a função do menu, agora passando a função de fade
            current_state = show_main_menu(display, queue, font_titulo_grande, font_botao, perform_fade);
            // Para a música do menu ao sair dele
            if (menu_instance) al_stop_sample_instance(menu_instance);
        }

        else if (current_state == STATE_GAMEPLAY)
        {
            reset_game_state(); // Reseta o jogo para uma nova partida
            al_start_timer(timer); // Inicia o timer do jogo

            // Escolhe uma música aleatória para a partida
            int track_index = rand() % NUM_GAMEPLAY_TRACKS;
            ALLEGRO_SAMPLE_INSTANCE* chosen_music = gameplay_music_instances[track_index];



            current_state = run_gameplay_loop(display, queue, timer, font_titulo_grande, font_subtitulo, font_botao, censure_sprite, knife_frame, death_sprite_frame, hit_instance, chosen_music, item_collect_instance, voice_instances, perform_fade);

            al_stop_timer(timer); // Para o timer ao voltar para o menu


        }
        else if (current_state == STATE_INSTRUCTIONS)
        {
            current_state = show_instructions_screen(display, queue, font_titulo_grande, font_subtitulo, font_botao);
        }
        else if (current_state == STATE_EXITING)
        {
            running = false; // condição para sair do loop principal
        }
    }

    // Limpeza
    al_destroy_bitmap(knife_frame);
    al_destroy_bitmap(floor_sprite);
    al_destroy_bitmap(top_right_shadow_floor_sprite);
    al_destroy_bitmap(top_left_shadow_floor_sprite);
    al_destroy_bitmap(top_shadow_floor_sprite);
    al_destroy_bitmap(right_shadow_floor_sprite);
    al_destroy_bitmap(left_shadow_floor_sprite);
    al_destroy_bitmap(wall_sprite);
    al_destroy_bitmap(mia_sprite);
    al_destroy_bitmap(lucia_sprite);
    al_destroy_bitmap(bruno_sprite);
    al_destroy_bitmap(miguel_sprite);
    al_destroy_bitmap(fernando_sprite);
    al_destroy_bitmap(julia_sprite);
    al_destroy_bitmap(lucas_sprite);
    al_destroy_bitmap(monica_sprite);
    al_destroy_bitmap(jay_sprite);
    al_destroy_bitmap(amanda_sprite);
    al_destroy_bitmap(marcos_sprite);
    al_destroy_bitmap(nick_sprite);
    al_destroy_bitmap(judite_sprite);
    al_destroy_bitmap(daniel_sprite);
    al_destroy_bitmap(fisky_sprite);
    al_destroy_bitmap(knife_sprite);
    al_destroy_bitmap(death_sprite_frame);
    al_destroy_bitmap(censure_sprite);
    al_destroy_bitmap(mirror_sprite);
    al_destroy_bitmap(light_mask);
    al_destroy_bitmap(light_buffer);
    al_destroy_sample_instance(hit_instance);
    al_destroy_sample(hit_sample);
    al_destroy_sample(gameplay_music_samples);
    al_destroy_sample_instance(gameplay_music_instances);
    al_destroy_sample(item_collect_sample);
    al_destroy_sample_instance(item_collect_instance);
    al_destroy_sample(voice_samples);
    al_destroy_sample_instance(voice_instances);
    al_destroy_sample(menu_music);
    al_destroy_sample_instance(menu_instance);
    al_destroy_font(font_titulo_grande);
    al_destroy_font(font_subtitulo);
    al_destroy_font(font_botao);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}
