//wall.png valor 0.
//left_shadow_floor.png (3 variações) valor 1.
//right_shadow_floor.png (3 variações) valor 2.
//top_shadow_floor.png (2 variações) valor 3.
//top_left_shadow_floor.png (40x40 pixels, 1 variação) valor 4.
//top_right_shadow_floor.png (40x40 pixels, 1 variação) valor 5.
//floor.png (6 variações) valor 6.

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
#define MAP_WIDTH 1280
#define MAP_HEIGHT 960
#define NUM_NPCS 15
#define MIN_NPC_DISTANCE 800
#define MAX_NPC_DISTANCE 1200
#define MIN_MIRROR_DISTANCE 500 // Nova constante para o espelho
#define TILE_SIZE 40          // Tamanho dos tiles
#define TILE_COLS (MAP_WIDTH / TILE_SIZE)  // 32 colunas
#define TILE_ROWS (MAP_HEIGHT / TILE_SIZE) // 24 linhas
#define CHANCE_TO_PURSUE 7    // Chance de um NPC começar a perseguir
#define LIGHT_RADIUS 125      // Raio do círculo de luz
#define AMBIENT_DARKNESS 0.95f  // Escuridão do ambiente (0.0 = claro, 1.0 = preto)


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

typedef struct {
    float x, y;              // Posição da faca (relativa ao jogador)
    int frame;               // Quadro da animação
    bool active;             // Se a faca ta sendo exibida
    float anim_timer;        // Temporizador para controlar a animação
    ALLEGRO_BITMAP *sprite_sheet; // Sprite da faca
} Knife;

// STRUCT PRA ITENS
typedef struct {
    float x, y;
    bool active;
    ALLEGRO_BITMAP *sprite;
} Item;


    int map[TILE_ROWS][TILE_COLS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,4,3,3,3,3,5,0,4,3,3,3,3,5,0,4,3,3,3,3,3,3,5,0,4,3,3,3,3,3,5,0},
        {0,1,6,2,0,1,2,0,0,0,1,2,0,0,0,0,0,0,0,0,1,2,0,0,1,6,6,6,6,6,2,0},
        {0,1,0,2,0,1,2,0,4,3,6,6,3,5,0,4,3,3,3,3,3,3,3,3,6,0,0,0,0,0,2,0},
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
        {0,1,2,0,0,2,5,0,4,2,0,0,1,0,4,3,5,0,0,0,1,2,0,0,0,0,2,0,1,0,2,0},
        {0,1,6,5,0,2,6,3,6,6,3,3,6,3,6,6,6,3,3,3,6,6,3,3,3,3,6,6,6,6,2,0},
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

// Variável global para a máscara de luz ---
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

    // Normaliza o vetor de direção
        float step_x = dx / distance;
        float step_y = dy / distance;

        // Define um tamanho de passo para a verificação.
        // Um valor menor é mais preciso, mas mais lento. Metade do tamanho de um tile é um bom começo.
        float step_size = TILE_SIZE / 2.0f;

        // "Caminha" ao longo da linha do ponto 1 para o ponto 2
        for (float i = 0; i < distance; i += step_size) {
            float current_x = x1 + i * step_x;
            float current_y = y1 + i * step_y;

        // Pega as coordenadas do tile no ponto atual da linha
            int tile_x = (int)(current_x / TILE_SIZE);
            int tile_y = (int)(current_y / TILE_SIZE);

        // Verifica se as coordenadas estão dentro do mapa
            if (tile_x < 0 || tile_x >= TILE_COLS || tile_y < 0 || tile_y >= TILE_ROWS) {
                return false; // Fora do mapa é considerado bloqueado
            }

        // Se o tile atual é uma parede (valor 0), a linha de visão está bloqueada.
            if (map[tile_y][tile_x] == 0) {
                return false;
            }
        }

    // Se loop terminar = nenhuma parede encontrada no caminho.
    return true;
    }


// Função para encontrar uma posição de spawn válida
    typedef struct {
        int row, col;
    } TilePosition;

    void find_valid_spawn_position(float *x, float *y, float player_x, float player_y, bool is_clone) {
        static TilePosition valid_tiles[TILE_ROWS * TILE_COLS];
        static int valid_tile_count = 0;
        static bool initialized = false;

    // Inicializar lista de tiles válidos
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

    if (is_clone) {
        // Para o clone, spawn a uma distância específica
        int attempts = 0;
        const int max_attempts = 50;
        while (attempts < max_attempts) {
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

    // Escolher um tile válido aleatoriamente
    int index = rand() % valid_tile_count;
    *x = valid_tiles[index].col * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
    *y = valid_tiles[index].row * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
}

// máscara de luz
    void create_light_mask() {
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

    ALLEGRO_FONT *font = al_load_ttf_font("../../fontes/arial.ttf", 20, 0);
    if (!font) {
        fprintf(stderr, "Aviso: Não foi possível carregar fonte '../../fontes/arial.ttf'. Continuando sem fonte.\n");
    }


    // luz
    light_buffer = al_create_bitmap(WIDTH, HEIGHT);
    create_light_mask(); // cria a máscara
    if (!light_buffer || !light_mask) {
        fprintf(stderr, "Erro ao criar recursos de iluminação.\n");
        return -1;
    }


    // Sons
    ALLEGRO_SAMPLE *hit_sample = NULL;
    ALLEGRO_SAMPLE_INSTANCE *hit_instance = NULL;
    ALLEGRO_SAMPLE *bg_music = NULL;
    ALLEGRO_SAMPLE_INSTANCE *bg_instance = NULL;

    if (al_install_audio())
    {
        al_reserve_samples(2); // Hit + música de fundo

        hit_sample = al_load_sample("../../sons/hit.wav");
        if (!hit_sample) {
            fprintf(stderr, "Aviso: Não foi possível carregar som '../../sons/hit.wav'. Continuando sem som de hit.\n");
        } else {
            hit_instance = al_create_sample_instance(hit_sample);
            if (hit_instance) {
                al_attach_sample_instance_to_mixer(hit_instance, al_get_default_mixer());
            } else {
                fprintf(stderr, "Aviso: Não foi possível criar instância de som de hit.\n");
            }
        }

        bg_music = al_load_sample("../../sons/lavander.wav");
        if (!bg_music) {
            fprintf(stderr, "Aviso: Não foi possível carregar música '../../sons/lavander.wav'. Continuando sem música.\n");
        } else {
            bg_instance = al_create_sample_instance(bg_music);
            if (bg_instance) {
                al_set_sample_instance_playmode(bg_instance, ALLEGRO_PLAYMODE_LOOP);
                al_attach_sample_instance_to_mixer(bg_instance, al_get_default_mixer());
                al_play_sample_instance(bg_instance);
            } else {
                fprintf(stderr, "Aviso: Não foi possível criar instância de música de fundo.\n");
            }
        }
    }

    // personagens
    ALLEGRO_BITMAP *mia_sprite = al_load_bitmap("../../sprites/mia.png");
    if (!mia_sprite) {
        fprintf(stderr, "Erro ao carregar sprite '../../sprites/mia.png'.\n");
        al_destroy_sample_instance(hit_instance);
        al_destroy_sample(hit_sample);
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }


    // **NOVO: Carregar sprites da nova mecânica**
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
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
        al_destroy_sample_instance(bg_instance);
        al_destroy_sample(bg_music);
        if (font) al_destroy_font(font);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return -1;
    }
    al_convert_mask_to_alpha(floor_sprite, al_map_rgb(255, 0, 255));


// Inicializar variações dos tiles
    srand(time(NULL));
    for (int row = 0; row < TILE_ROWS; row++) {
        for (int col = 0; col < TILE_COLS; col++) {
            switch (map[row][col]) {
                case 1: tile_variations[row][col] = rand() % 3; break;
                case 2: tile_variations[row][col] = rand() % 3; break;
                case 3: tile_variations[row][col] = rand() % 2; break;
                case 6: tile_variations[row][col] = rand() % 6; break;
                default: tile_variations[row][col] = 0; break;
            }
        }
    }

    // Inicializar personagens
    ALLEGRO_BITMAP *character_sprites[NUM_NPCS] = {
        mia_sprite, lucia_sprite, bruno_sprite, miguel_sprite, fernando_sprite, julia_sprite, lucas_sprite, monica_sprite, jay_sprite, amanda_sprite, marcos_sprite, nick_sprite, judite_sprite, daniel_sprite, fisky_sprite
    };

    Entity player;
    find_valid_spawn_position(&player.x, &player.y, 0, 0, false); // Passando 0,0 pois não precisa de referência
    player.frame = 0;
    player.movement = 0;
    player.alive = true;
    player.is_target = false;
    player.sprite_sheet = character_sprites[rand() % NUM_NPCS];
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

    Entity npcs[NUM_NPCS];
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
        npcs[i].death_timer = 0.0f; // NOVO: Inicializa o timer em 0.
    }

    // Inicializar faca
    Knife knife;
    knife.x = 0;
    knife.y = 0;
    knife.frame = 0;
    knife.active = false;
    knife.anim_timer = 0;
    knife.sprite_sheet = knife_sprite;

    // inicializar espelho**
    Item mirror_item;
    mirror_item.sprite = mirror_sprite;
    mirror_item.active = true;

    // posição pra o espelho a uma distância mínima
    int attempts = 0;
    while(attempts < 100) { // Evita loop infinito
        float spawn_x, spawn_y;
        find_valid_spawn_position(&spawn_x, &spawn_y, 0, 0, false);
        float dx = spawn_x - player.x;
        float dy = spawn_y - player.y;
        if (sqrt(dx*dx + dy*dy) >= MIN_MIRROR_DISTANCE) {
            mirror_item.x = spawn_x;
            mirror_item.y = spawn_y;
            break;
        }
        attempts++;
    }
    if (attempts >= 100) { // se não encontrar lugar
        find_valid_spawn_position(&mirror_item.x, &mirror_item.y, 0, 0, false);
    }


    // Eventos
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool running = true, redraw = true;
    double sprite_timer = 0.0, sprite_delay = 0.15;
    bool keys[4] = { false, false, false, false };
    float camera_x = player.x - WIDTH / 2, camera_y = player.y - HEIGHT / 2;
    bool game_over = false;
    bool victory = false;
    bool found_mirror = false; // **NOVO: Variável de estado da mecânica**

    al_start_timer(timer);


// Looping principal
    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        switch (ev.type) { // tipo de evento
            case ALLEGRO_EVENT_DISPLAY_CLOSE: //fechar jogo
                running = false;
                break;

            case ALLEGRO_EVENT_KEY_DOWN: //quando teclas são pressionadas
                if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) running = false;
                if (ev.keyboard.keycode == ALLEGRO_KEY_W) keys[0] = true;
                if (ev.keyboard.keycode == ALLEGRO_KEY_S) keys[1] = true;
                if (ev.keyboard.keycode == ALLEGRO_KEY_A) keys[2] = true;
                if (ev.keyboard.keycode == ALLEGRO_KEY_D) keys[3] = true;
                break;

            case ALLEGRO_EVENT_KEY_UP: // quando teclas são soltas
                if (ev.keyboard.keycode == ALLEGRO_KEY_W) keys[0] = false;
                if (ev.keyboard.keycode == ALLEGRO_KEY_S) keys[1] = false;
                if (ev.keyboard.keycode == ALLEGRO_KEY_A) keys[2] = false;
                if (ev.keyboard.keycode == ALLEGRO_KEY_D) keys[3] = false;
                break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: // clique do mouse
                if (ev.mouse.button == 1 && !game_over && !knife.active)
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
                    for (int i = 0; i < NUM_NPCS; i++)
                    {
                        // Checa se o NPC está vivo (e não já morrendo)
                        if (npcs[i].alive) {
                            float npc_center_x = npcs[i].x + SPRITE_SIZE / 2;
                            float npc_center_y = npcs[i].y + SPRITE_SIZE / 2;
                            float player_center_x = player.x + SPRITE_SIZE / 2;
                            float player_center_y = player.y + SPRITE_SIZE / 2;

                            float dx = npc_center_x - (knife.x + KNIFE_WIDTH / 2);
                            float dy = npc_center_y - (knife.y + KNIFE_HEIGHT / 2);

                            if (dx * dx + dy * dy < KNIFE_RANGE * KNIFE_RANGE &&
                                has_line_of_sight(player_center_x, player_center_y, npc_center_x, npc_center_y))
                            {
                                // Em vez de só matar, agora iniciamos a animação de morte.
                                npcs[i].alive = false;          // Para a lógica de movimento e IA do NPC.
                                npcs[i].death_timer = 1.0f;     // NOVO: Inicia um contador de 1 segundo para a animação.

                                if (npcs[i].is_target) {
                                    printf("Clone eliminado! Vitoria!\n");
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

                       case ALLEGRO_EVENT_TIMER: // controle das animações
                sprite_timer += 1.0 / 60.0;
                if (sprite_timer >= sprite_delay) {
                    player.frame = (player.frame + 1) % SPRITE_COLS;
                    for (int i = 0; i < NUM_NPCS; i++)
                        if (npcs[i].alive)
                            npcs[i].frame = (npcs[i].frame + 1) % SPRITE_COLS;
                    sprite_timer = 0.0;
                }

                if (knife.active) { // fazer animação da faca acontecer e depois desaparecer
                    knife.anim_timer += 1.0 / 60.0;
                    if (knife.anim_timer >= KNIFE_ANIM_SPEED) {
                        knife.frame++;
                        knife.anim_timer = 0;
                        if (knife.frame >= KNIFE_FRAMES) {
                            knife.active = false;
                            knife.frame = 0;
                        }
                    }
                }

                if (!game_over) // tudo do jogo só acontece se o jogo nao tiver acabado
                {
                    float speed = 3.0;
                    float new_x = player.x;
                    float new_y = player.y;

                    if (keys[0]) { new_y -= speed; player.movement = 3; } // w
                    if (keys[1]) { new_y += speed; player.movement = 0; } // a
                    if (keys[2]) { new_x -= speed; player.movement = 1; } // s
                    if (keys[3]) { new_x += speed; player.movement = 2; } // d

                    if (can_move(new_x, new_y)) { // nao passar paredes
                        player.x = new_x;
                        player.y = new_y;
                    }

                    //  manter o jogador e a câmera dentro da área de jogo
                    if (player.x < 0) player.x = 0;
                    if (player.x > MAP_WIDTH - SPRITE_SIZE) player.x = MAP_WIDTH - SPRITE_SIZE;
                    if (player.y < 0) player.y = 0;
                    if (player.y > MAP_HEIGHT - SPRITE_SIZE) player.y = MAP_HEIGHT - SPRITE_SIZE;

                    camera_x = player.x - WIDTH / 2;
                    camera_y = player.y - HEIGHT / 2;
                    if (camera_x < 0) camera_x = 0;
                    if (camera_x > MAP_WIDTH - WIDTH) camera_x = MAP_WIDTH - WIDTH;
                    if (camera_y < 0) camera_y = 0;
                    if (camera_y > MAP_HEIGHT - HEIGHT) camera_y = MAP_HEIGHT - HEIGHT;

                    if (mirror_item.active) { // se espelho ainda no mapa
                        float dx_mirror = player.x - mirror_item.x;
                        float dy_mirror = player.y - mirror_item.y;
                        if (sqrt(dx_mirror * dx_mirror + dy_mirror * dy_mirror) < (SPRITE_SIZE / 2)) {
                            found_mirror = true;
                            mirror_item.active = false;
                            printf("Voce encontrou o espelho!\n");
                        }
                    }

                    // desvio de paredes
                    for (int i = 0; i < NUM_NPCS; i++) {

                        if (npcs[i].alive) { // verificação se o npc está vivo e próximas decisões
                            float new_x_npc = npcs[i].x;
                            float new_y_npc = npcs[i].y;
                            float speed = npcs[i].is_target ? 2.0 : 1.5;
                            bool is_pursuing = npcs[i].is_target;

                            if (!npcs[i].is_target) {
                                npcs[i].move_timer -= 1.0 / 60.0;
                                if (npcs[i].move_timer <= 0) {
                                    npcs[i].move_timer = (rand() % 60 + 40) / 10.0;
                                    if ((rand() % 100) < CHANCE_TO_PURSUE) npcs[i].move_mode = -1;
                                    else npcs[i].move_mode = rand() % 4;
                                }
                                if (npcs[i].move_mode == -1) is_pursuing = true;
                            }

                            if (is_pursuing)  // verificação se o npc a vivo e proximas decisões
                            {
                                float dx = player.x - npcs[i].x;
                                float dy = player.y - npcs[i].y;
                                float dist = sqrt(dx * dx + dy * dy);

                                if (dist > 1) // calcula o vetor do jogador
                                {
                                    float move_x = (dx / dist) * speed;
                                    float move_y = (dy / dist) * speed;

                                    if (can_move(npcs[i].x + move_x, npcs[i].y + move_y)) {
                                        new_x_npc += move_x; new_y_npc += move_y; // Caminho ideal
                                    } else // caso não haja
                                    {
                                        if (can_move(npcs[i].x + move_x, npcs[i].y)) new_x_npc += move_x; // mover em x
                                        else if (can_move(npcs[i].x, npcs[i].y + move_y)) new_y_npc += move_y; // se não, mover em y
                                        else if (!npcs[i].is_target) npcs[i].move_timer = 0; // se não, forcar nova decisao
                                    }
                                }
                                float angle = atan2(dy, dx) * 180 / M_PI;
                                    if (angle >= -45 && angle < 45) npcs[i].movement = 2; // direita
                                    else if (angle >= 45 && angle < 135) npcs[i].movement = 0; // baixo
                                    else if (angle >= 135 || angle < -135) npcs[i].movement = 1; // esquerda
                                    else npcs[i].movement = 3; // cima

                            } else //se houver colisão, força nova decisão
                            {
                                switch (npcs[i].move_mode) {
                                    case 0: new_y_npc -= speed; npcs[i].movement = 3; break;
                                    case 1: new_y_npc += speed; npcs[i].movement = 0; break;
                                    case 2: new_x_npc -= speed; npcs[i].movement = 1; break;
                                    case 3: new_x_npc += speed; npcs[i].movement = 2; break;
                                }
                                if (!can_move(new_x_npc, new_y_npc)) {
                                    npcs[i].move_timer = 0; new_x_npc = npcs[i].x; new_y_npc = npcs[i].y;
                                }
                            }
                            // atualizaaar posições
                            npcs[i].x = new_x_npc;
                            npcs[i].y = new_y_npc;

                            // lógica do hit  não funcionar entre paredes
                            if (npcs[i].is_target) {
                                float final_dist = sqrt(pow(player.x - npcs[i].x, 2) + pow(player.y - npcs[i].y, 2));
                                float player_center_x = player.x + SPRITE_SIZE / 2;
                                float player_center_y = player.y + SPRITE_SIZE / 2;
                                float npc_center_x = npcs[i].x + SPRITE_SIZE / 2;
                                float npc_center_y = npcs[i].y + SPRITE_SIZE / 2;
                                if (final_dist < 50 && !game_over && has_line_of_sight(player_center_x, player_center_y, npc_center_x, npc_center_y)) {
                                    game_over = true;
                                    victory = false;
                                }
                            }
                            if (npcs[i].x < 0) npcs[i].x = 0;
                        }
                        // Se o NPC não está vivo, ver se ele ta na animação de morte.
                        else if (npcs[i].death_timer > 0) {
                            // Diminui o contador a cada frame. (1.0/60.0 por segundo)
                            npcs[i].death_timer -= 1.0 / 60.0;
                        }
                    }
                }
                redraw = true;
                break;
        }

        if (redraw && al_is_event_queue_empty(queue))
        {
            redraw = false;
            al_set_target_bitmap(light_buffer);
            al_clear_to_color(al_map_rgb(0, 0, 0));

            for (int row = 0; row < TILE_ROWS; row++) {
                for (int col = 0; col < TILE_COLS; col++) {
                    float draw_x = col * TILE_SIZE - camera_x;
                    float draw_y = row * TILE_SIZE - camera_y;
                    if (draw_x >= -80 && draw_x < WIDTH && draw_y >= -80 && draw_y < HEIGHT) {
                        int tile_type = map[row][col];
                        int variation = tile_variations[row][col];
                        switch (tile_type) {
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
            }

        // Desenhar espelho
            if (mirror_item.active) {
                al_draw_bitmap(mirror_item.sprite, mirror_item.x - camera_x, mirror_item.y - camera_y, 0);
            }

            if (player.alive && player.sprite_sheet) {
                int sprite_x = player.frame * SPRITE_SIZE;
                int sprite_y = player.movement * SPRITE_SIZE;
                al_draw_bitmap_region(player.sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, player.x - camera_x, player.y - camera_y, 0);

                // desenhar censura quando espelho não encontrado
                if (!found_mirror && censure_sprite) {
                    al_draw_bitmap(censure_sprite, player.x - camera_x, player.y - camera_y, 0);
                }
            }

        // Desenhar faca
            if (knife.active && knife.sprite_sheet) // Fatores a se considerar
            {
                int sprite_x = knife.frame * KNIFE_WIDTH;
                int sprite_y = 0;
                float draw_x = knife.x - camera_x;
                float draw_y = knife.y - camera_y;
                al_set_target_bitmap(knife_frame);
                al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                al_draw_bitmap_region(knife.sprite_sheet, sprite_x, sprite_y, KNIFE_WIDTH, KNIFE_HEIGHT, 0, 0, 0);
                al_set_target_bitmap(light_buffer);

                switch (player.movement) { // rotação da faca
                    case 0: al_draw_rotated_bitmap(knife_frame, KNIFE_WIDTH / 2, KNIFE_HEIGHT / 2, draw_x + KNIFE_WIDTH / 2, draw_y + KNIFE_HEIGHT / 2, M_PI / 2, 0); break;
                    case 1: al_draw_bitmap_region(knife.sprite_sheet, sprite_x, sprite_y, KNIFE_WIDTH, KNIFE_HEIGHT, draw_x, draw_y, ALLEGRO_FLIP_HORIZONTAL); break;
                    case 2: al_draw_bitmap_region(knife.sprite_sheet, sprite_x, sprite_y, KNIFE_WIDTH, KNIFE_HEIGHT, draw_x, draw_y, 0); break;
                    case 3: al_draw_rotated_bitmap(knife_frame, KNIFE_WIDTH / 2, KNIFE_HEIGHT / 2, draw_x + KNIFE_WIDTH / 2, draw_y + KNIFE_HEIGHT / 2, -M_PI / 2, 0); break;
                }
            }


        // NPC
            for (int i = 0; i < NUM_NPCS; i++)
            {
                // Desenha se estiver vivo OU na animação de morte.
                if ((npcs[i].alive || npcs[i].death_timer > 0) && npcs[i].sprite_sheet) {

                    if (npcs[i].alive) {
                        // Lógica de desenho p npc vivo
                        int sprite_x = npcs[i].frame * SPRITE_SIZE;
                        int sprite_y = npcs[i].movement * SPRITE_SIZE;
                        al_draw_bitmap_region(npcs[i].sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, npcs[i].x - camera_x, npcs[i].y - camera_y, 0);
                    } else {

                        // Lógica de desenho para a ANIMAÇÃO DE MORTE.
                        // Posições e frames
                        int sprite_x = 0; // Usa o primeiro frame (parado)
                        int sprite_y = 0; // Usa a direção para baixo
                        float draw_x = npcs[i].x - camera_x;
                        float draw_y = npcs[i].y - camera_y;

                        // 1. Salva o alvo atual e desenha o frame do NPC no bitmap temporário.
                        ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();
                        al_set_target_bitmap(death_sprite_frame);
                        al_clear_to_color(al_map_rgba(0, 0, 0, 0)); // Limpa o bitmap temporário
                        al_draw_bitmap_region(npcs[i].sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, 0, 0, 0);
                        al_set_target_bitmap(prev_target); // Restaura o alvo original (light_buffer)

                        // 2. Desenha o bitmap temporário na tela
                        al_draw_tinted_rotated_bitmap
                        (
                            death_sprite_frame,     // O bitmap temporário
                            al_map_rgb(255, 120, 120),      // Filtro de cor (vermelho)
                            SPRITE_SIZE / 2.0f, SPRITE_SIZE / 2.0f,     // ponto para rotação
                            draw_x + SPRITE_SIZE / 2.0f, draw_y + SPRITE_SIZE / 2.0f, // posição de destino na tela
                            M_PI / 2.0f,                                  // Ângulo em radianos
                            0                                             // Flags (nenhum)
                        );
                    }
                }
            }
            al_set_target_backbuffer(display);
            al_clear_to_color(al_map_rgb(0, 0, 0));


        // ILUMINAÇÃO
            if (light_mask) {
                al_draw_tinted_bitmap(light_buffer, al_map_rgb_f(1.0 - AMBIENT_DARKNESS, 1.0 - AMBIENT_DARKNESS, 1.0 - AMBIENT_DARKNESS), 0, 0, 0);
                al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
                al_draw_bitmap_region(light_buffer,
                            player.x - camera_x + (SPRITE_SIZE / 2.0f) - LIGHT_RADIUS, player.y - camera_y + (SPRITE_SIZE / 2.0f) - LIGHT_RADIUS,
                            LIGHT_RADIUS * 2, LIGHT_RADIUS * 2,
                            player.x - camera_x + (SPRITE_SIZE / 2.0f) - LIGHT_RADIUS, player.y - camera_y + (SPRITE_SIZE / 2.0f) - LIGHT_RADIUS,
                            0);
                al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
            }

            if (game_over && font) {
                const char *msg = victory ? "Você venceu!" : "Você perdeu!";
                al_draw_text(font, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTER, msg);
            } else if (game_over) {
                printf("Fim de jogo: %s\n", victory ? "Vitória!" : "Derrota!");
            }

            al_flip_display();
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
    al_destroy_sample_instance(bg_instance);
    al_destroy_sample(bg_music);
    if (font) al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}
