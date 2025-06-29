# Kauany
// Meu trabalho final

/*
 * Feito por Thiago e Kauany.
 * * JOGO: ROCK N' ROLL RACING - TRABALHO PRÁTICO FINAL
*/

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

//------------------------------------------------------------------------------------
// DEFINIÇÕES GLOBAIS (CONSTANTES)
//------------------------------------------------------------------------------------

// --- Configurações do Mapa e da Pista ---
#define MAP_LARGURA 80
#define MAP_ALTURA  20
#define TOTAL_VOLTAS 3
#define FATOR_ESCALA 12

// --- Configurações da Janela e do HUD ---
#define TELA_LARGURA 1200
#define TELA_ALTURA 600
#define GAME_AREA_LARGURA (MAP_LARGURA * FATOR_ESCALA)
#define GAME_AREA_ALTURA  (MAP_ALTURA * FATOR_ESCALA)
#define ALTURA_HUD 40

// --- Física e Controlo dos Carros ---
#define ACELERACAO        0.1f
#define VELOCIDADE_MAXIMA 5.0f
#define ATRITO            0.03f
#define VELOCIDADE_ROTACAO  5.0f
#define DANO_PAREDE       5
#define DANO_TIRO         50
#define DANO_BOMBA_REVES  50

// --- Inteligência Artificial ---
#define DISTANCIA_SENSOR_IA 35.0f

// --- Limites de Entidades ---
#define MAX_INIMIGOS 3
#define MAX_CAIXAS   5
#define MAX_TIROS    10

// --- Cores ---
#define DARK_BLUE (Color){ 0, 12, 33, 79 }

//------------------------------------------------------------------------------------
// ESTRUTURAS DE DADOS E TIPOS
//------------------------------------------------------------------------------------

typedef enum { ESTADO_MENU, ESTADO_JOGO, ESTADO_FIM, ESTADO_CONFIRMA_SAIR, ESTADO_SAIR } EstadoJogo;
typedef enum { ITEM_ARMA, ITEM_ESCUDO, ITEM_BOMBA_REVES } TipoItem;

typedef struct Carro {
    Vector2 posicao, velocidade;
    float angulo;
    int vida, voltas_completas, dano_timer;
    int ia_reversing_timer;
    bool na_linha_de_chegada_anterior, tem_arma, tem_escudo;
    Color cor;
} Carro;

typedef struct CaixaItem {
    Rectangle retangulo;
    bool ativa;
    TipoItem tipo;
} CaixaItem;

typedef struct Tiro {
    Vector2 posicao, velocidade;
    bool ativo;
} Tiro;

typedef struct JogoSalvo {
    Carro jogador, inimigos[MAX_INIMIGOS];
    CaixaItem caixas[MAX_CAIXAS];
    char nome_mapa[50];
    int numInimigos;
} JogoSalvo;

//------------------------------------------------------------------------------------
// FUNÇÕES MATEMÁTICAS AUXILIARES
//------------------------------------------------------------------------------------

static Vector2 MeuVector2Zero(void) { return (Vector2){0}; }
static Vector2 MeuVector2Add(Vector2 v1, Vector2 v2) { return (Vector2){v1.x + v2.x, v1.y + v2.y}; }
static Vector2 MeuVector2Subtract(Vector2 v1, Vector2 v2) { return (Vector2){v1.x - v2.x, v1.y - v2.y}; }
static Vector2 MeuVector2Scale(Vector2 v, float scalar) { return (Vector2){v.x * scalar, v.y * scalar}; }
static float MeuVector2Length(Vector2 v) { return sqrtf(v.x * v.x + v.y * v.y); }
static Vector2 MeuVector2Normalize(Vector2 v) {
    float length = MeuVector2Length(v);
    if (length > 0) return MeuVector2Scale(v, 1.0f / length);
    return v;
}

//------------------------------------------------------------------------------------
// PROTÓTIPOS DAS FUNÇÕES DO JOGO
//------------------------------------------------------------------------------------

void menu(void);
void jogo(bool carregarJogo);
void CarregarMapaDoFicheiro(const char *nomeFicheiro, char mapa[MAP_ALTURA][MAP_LARGURA + 1]);
void DesenharMapa(const char mapa[MAP_ALTURA][MAP_LARGURA + 1]);
Vector2 EncontrarPosicaoInicial(const char mapa[MAP_ALTURA][MAP_LARGURA + 1], char simbolo);
void InicializarJogo(Carro *jogador, Carro inimigos[], CaixaItem caixas[], const char mapa[MAP_ALTURA][MAP_LARGURA + 1], int *numInimigos);
void AtualizarJogo(Carro *jogador, Carro inimigos[], CaixaItem caixas[], const char mapa[MAP_ALTURA][MAP_LARGURA + 1], Tiro tiros[], int numInimigos, EstadoJogo *estadoAtual, char *mensagemFim, int *feedbackTimer);
void AtualizarInimigoIA(Carro *inimigo, const char mapa[MAP_ALTURA][MAP_LARGURA + 1]);
void DesenharJogo(const Carro *jogador, const Carro inimigos[], const CaixaItem caixas[], const char mapa[MAP_ALTURA][MAP_LARGURA + 1], int numInimigos, const Tiro tiros[], const char *feedbackMessage, int feedbackTimer);
void DesenharHUD(const Carro *jogador);
void SalvarJogo(const JogoSalvo *jogo);
bool CarregarJogo(JogoSalvo *jogo);

//----------------------------------------------------------------------------------
// FUNÇÃO PRINCIPAL (main)
//----------------------------------------------------------------------------------

int main(void) {
    InitWindow(TELA_LARGURA, TELA_ALTURA, "Rock N Roll Racing - by Thiago e Kauany");
    SetTargetFPS(60);

    menu();

    CloseWindow();

    return 0;
}

//----------------------------------------------------------------------------------
// LÓGICA DO MENU PRINCIPAL
//----------------------------------------------------------------------------------

void menu(void) {
    bool sair_do_menu = false;

    while (!sair_do_menu && !WindowShouldClose()) {

        if(IsKeyPressed(KEY_N)){
            jogo(false);
        } else if(IsKeyPressed(KEY_C)){
            jogo(true);
        } else if(IsKeyPressed(KEY_Q)){
            bool confirma_saida = false;
            bool volta_menu = true;
            while(volta_menu && !WindowShouldClose()){
                BeginDrawing();
                    ClearBackground(DARK_BLUE);
                    int textX = (TELA_LARGURA - MeasureText("Tem certeza que deseja fechar o jogo?", 45)) / 2;
                    int textY = (TELA_ALTURA - 200) / 2;
                    DrawText("Tem certeza que deseja fechar o jogo?", textX, textY, 45, WHITE);
                    textX = (TELA_LARGURA - MeasureText("(F) Fechar jogo  |  (V) Voltar para o Menu", 45)) / 2;
                    DrawText("(F) Fechar jogo  |  (V) Voltar para o Menu", textX, textY + 100, 45, WHITE);
                EndDrawing();
                if(IsKeyPressed(KEY_V)){
                    volta_menu = false;
                }else if(IsKeyPressed(KEY_F)){
                    confirma_saida = true;
                    volta_menu = false;
                }
            }
            if (confirma_saida) {
                sair_do_menu = true;
            }
        }

        BeginDrawing();
            ClearBackground(DARK_BLUE);
            int textX = (TELA_LARGURA - MeasureText("Rock N Roll Racing", 70)) / 2;
            DrawText("Rock N Roll Racing", textX, 80, 70, RED);

            textX = (TELA_LARGURA - MeasureText("(N) Novo Jogo", 40)) / 2;
            DrawText("(N) Novo Jogo", textX, 200, 40, WHITE);
            textX = (TELA_LARGURA - MeasureText("(X) Salvar Jogo", 40)) / 2;
            DrawText("(X) Salvar Jogo", textX, 300, 40, GRAY);
            textX = (TELA_LARGURA - MeasureText("(C) Carregar Jogo", 40)) / 2;
            DrawText("(C) Carregar Jogo", textX, 400, 40, WHITE);
            textX = (TELA_LARGURA - MeasureText("(Q) Sair do Jogo", 40)) / 2;
            DrawText("(Q) Sair do Jogo", textX, 500, 40, WHITE);

        EndDrawing();
    }
}


//----------------------------------------------------------------------------------
// LÓGICA DO JOGO (Corrida)
//----------------------------------------------------------------------------------
void jogo(bool carregarJogo) {
    EstadoJogo estadoAtual = ESTADO_JOGO;
    char mensagemFim[50] = "";
    char feedbackMessage[50] = "";
    int feedbackTimer = 0;

    char mapa[MAP_ALTURA][MAP_LARGURA + 1];
    Carro jogador, inimigos[MAX_INIMIGOS];
    CaixaItem caixas[MAX_CAIXAS];
    Tiro tiros[MAX_TIROS] = {0};
    int numInimigos = 0;
    JogoSalvo estadoParaSalvar = {0};
    char nomeMapaAtual[50] = "mapa-01.txt";

    if (carregarJogo && CarregarJogo(&estadoParaSalvar)) {
        strcpy(nomeMapaAtual, estadoParaSalvar.nome_mapa);
        CarregarMapaDoFicheiro(nomeMapaAtual, mapa);
        jogador = estadoParaSalvar.jogador;
        memcpy(inimigos, estadoParaSalvar.inimigos, sizeof(inimigos));
        memcpy(caixas, estadoParaSalvar.caixas, sizeof(caixas));
        numInimigos = estadoParaSalvar.numInimigos;
        strcpy(feedbackMessage, "JOGO CARREGADO!");
        feedbackTimer = 120;
    } else {
        CarregarMapaDoFicheiro(nomeMapaAtual, mapa);
        InicializarJogo(&jogador, inimigos, caixas, mapa, &numInimigos);
    }

    while (estadoAtual == ESTADO_JOGO && !WindowShouldClose()) {

        if (IsKeyPressed(KEY_X)) {
            estadoParaSalvar.jogador = jogador;
            memcpy(estadoParaSalvar.inimigos, inimigos, sizeof(inimigos));
            memcpy(estadoParaSalvar.caixas, caixas, sizeof(caixas));
            strcpy(estadoParaSalvar.nome_mapa, nomeMapaAtual);
            estadoParaSalvar.numInimigos = numInimigos;
            SalvarJogo(&estadoParaSalvar);
            strcpy(feedbackMessage, "JOGO SALVO!");
            feedbackTimer = 120;
        }

        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
            estadoAtual = ESTADO_MENU;
        }

        if(estadoAtual == ESTADO_JOGO) {
            AtualizarJogo(&jogador, inimigos, caixas, mapa, tiros, numInimigos, &estadoAtual, mensagemFim, &feedbackTimer);

            BeginDrawing();
                ClearBackground((Color){50, 50, 50, 255});
                DesenharJogo(&jogador, inimigos, caixas, mapa, numInimigos, tiros, feedbackMessage, feedbackTimer);
            EndDrawing();
        }

        if (estadoAtual == ESTADO_FIM) {
             while (!WindowShouldClose() && !IsKeyPressed(KEY_ENTER)) {
                BeginDrawing();
                ClearBackground((Color){50, 50, 50, 255});
                DesenharJogo(&jogador, inimigos, caixas, mapa, numInimigos, tiros, feedbackMessage, feedbackTimer);
                DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.6f));
                DrawText(mensagemFim, TELA_LARGURA / 2 - MeasureText(mensagemFim, 40) / 2, TELA_ALTURA / 2 - 40, 40, YELLOW);
                DrawText("Pressione ENTER para voltar ao menu", TELA_LARGURA / 2 - MeasureText("Pressione ENTER para voltar ao menu", 20) / 2, TELA_ALTURA / 2 + 20, 20, WHITE);
                EndDrawing();
            }
        }
    }
}


//----------------------------------------------------------------------------------
// FUNÇÕES DE IMPLEMENTAÇÃO DO JOGO
//----------------------------------------------------------------------------------

void CarregarMapaDoFicheiro(const char *nomeFicheiro, char mapa[MAP_ALTURA][MAP_LARGURA + 1]) {
    FILE *arquivo = fopen(nomeFicheiro, "r");
    if (!arquivo) {
        printf("ERRO: Nao foi possivel abrir o ficheiro do mapa: %s\n", nomeFicheiro);
        for (int y = 0; y < MAP_ALTURA; y++) {
            for (int x = 0; x < MAP_LARGURA; x++) mapa[y][x] = ' ';
            mapa[y][MAP_LARGURA] = '\0';
        }
        return;
    }
    for (int y = 0; y < MAP_ALTURA; y++) {
        if (fgets(mapa[y], MAP_LARGURA + 2, arquivo) == NULL) {
             // Se não conseguir ler mais linhas, termina o loop
        } else {
            bool newlineEncontrada = false;
            for(int x = 0; x <= MAP_LARGURA && !newlineEncontrada; x++){
                if(mapa[y][x] == '\n' || mapa[y][x] == '\r') {
                    mapa[y][x] = '\0';
                    newlineEncontrada = true;
                }
            }
        }
    }
    fclose(arquivo);
}


void DesenharMapa(const char mapa[MAP_ALTURA][MAP_LARGURA + 1]) {
    for (int y = 0; y < MAP_ALTURA; y++) {
        for (int x = 0; x < MAP_LARGURA; x++) {
            Rectangle tile = {(float)x * FATOR_ESCALA, (float)y * FATOR_ESCALA, (float)FATOR_ESCALA, (float)FATOR_ESCALA};
            char simbolo = mapa[y][x];
            if (simbolo == 'j' || simbolo == 'i') simbolo = ' ';

            switch (simbolo) {
                case ' ':
                    DrawRectangleRec(tile, (Color){80, 80, 80, 255});
                    break;
                case 'P':
                case 'p':
                    DrawRectangleRec(tile, DARKGRAY);
                    DrawRectangleLinesEx(tile, 1.5, BLACK);
                    break;
                case 'L':
                    if ((x + y) % 2 == 0) DrawRectangleRec(tile, WHITE);
                    else DrawRectangleRec(tile, LIGHTGRAY);
                    DrawRectangleLinesEx(tile, 1.0, BLACK);
                    break;
                default: break;
            }
        }
    }
}

Vector2 EncontrarPosicaoInicial(const char mapa[MAP_ALTURA][MAP_LARGURA + 1], char simbolo) {
    Vector2 posicaoEncontrada = {-1, -1};
    bool encontrado = false;

    for (int y = 0; y < MAP_ALTURA && !encontrado; y++) {
        for (int x = 0; x < MAP_LARGURA && !encontrado; x++) {
            if (mapa[y][x] == simbolo) {
                posicaoEncontrada = (Vector2){(float)x * FATOR_ESCALA + FATOR_ESCALA / 2.0f, (float)y * FATOR_ESCALA + FATOR_ESCALA / 2.0f};
                encontrado = true;
            }
        }
    }
    return posicaoEncontrada;
}

void InicializarJogo(Carro *jogador, Carro inimigos[], CaixaItem caixas[], const char mapa[MAP_ALTURA][MAP_LARGURA + 1], int *numInimigos) {
    jogador->posicao = EncontrarPosicaoInicial(mapa, 'j');
    if (jogador->posicao.x == -1) jogador->posicao = (Vector2){100, 100};
    jogador->velocidade = MeuVector2Zero();
    jogador->angulo = 90.0f;
    jogador->vida = 100;
    jogador->voltas_completas = 0;
    jogador->na_linha_de_chegada_anterior = false;
    jogador->tem_arma = false;
    jogador->tem_escudo = false;
    jogador->dano_timer = 0;
    jogador->ia_reversing_timer = 0;
    jogador->cor = BLUE;

    *numInimigos = 0;
    Vector2 posInimigo = EncontrarPosicaoInicial(mapa, 'i');
    if (posInimigo.x != -1) {
        if (*numInimigos < MAX_INIMIGOS) {
            inimigos[*numInimigos].posicao = posInimigo;
            inimigos[*numInimigos].velocidade = MeuVector2Zero();
            inimigos[*numInimigos].angulo = 90.0f;
            inimigos[*numInimigos].vida = 100;
            inimigos[*numInimigos].voltas_completas = 0;
            inimigos[*numInimigos].na_linha_de_chegada_anterior = false;
            inimigos[*numInimigos].tem_arma = false;
            inimigos[*numInimigos].tem_escudo = false;
            inimigos[*numInimigos].dano_timer = 0;
            inimigos[*numInimigos].ia_reversing_timer = 0;
            inimigos[*numInimigos].cor = RED;
            (*numInimigos)++;
        }
    }

    for (int i = 0; i < MAX_CAIXAS; i++) {
        caixas[i].ativa = true;
        int randX, randY;
        do {
            randX = GetRandomValue(0, MAP_LARGURA - 1);
            randY = GetRandomValue(0, MAP_ALTURA - 1);
        } while (mapa[randY][randX] != ' ');

        caixas[i].retangulo = (Rectangle){(float)randX * FATOR_ESCALA, (float)randY * FATOR_ESCALA, (float)FATOR_ESCALA, (float)FATOR_ESCALA};
        caixas[i].tipo = (TipoItem)GetRandomValue(0, 2);
    }
}

void AtualizarInimigoIA(Carro *inimigo, const char mapa[MAP_ALTURA][MAP_LARGURA + 1]) {
    if (inimigo->dano_timer > 0) {
        inimigo->dano_timer--;
        return;
    }

    if (inimigo->ia_reversing_timer > 0) {
        Vector2 dirRe = {cosf(inimigo->angulo * DEG2RAD), sinf(inimigo->angulo * DEG2RAD)};
        inimigo->velocidade = MeuVector2Subtract(inimigo->velocidade, MeuVector2Scale(dirRe, ACELERACAO * 0.5f));
        inimigo->angulo += VELOCIDADE_ROTACAO * 2.0f;
        inimigo->ia_reversing_timer--;
    } else {
        Vector2 frente = {cosf(inimigo->angulo * DEG2RAD), sinf(inimigo->angulo * DEG2RAD)};
        Vector2 sensor_frente_pos = MeuVector2Add(inimigo->posicao, MeuVector2Scale(frente, DISTANCIA_SENSOR_IA));

        float angulo_sensor_lateral = 45.0f;
        Vector2 frente_esquerda = {cosf((inimigo->angulo - angulo_sensor_lateral) * DEG2RAD), sinf((inimigo->angulo - angulo_sensor_lateral) * DEG2RAD)};
        Vector2 frente_direita = {cosf((inimigo->angulo + angulo_sensor_lateral) * DEG2RAD), sinf((inimigo->angulo + angulo_sensor_lateral) * DEG2RAD)};
        Vector2 sensor_esquerda_pos = MeuVector2Add(inimigo->posicao, MeuVector2Scale(frente_esquerda, DISTANCIA_SENSOR_IA / 2));
        Vector2 sensor_direita_pos = MeuVector2Add(inimigo->posicao, MeuVector2Scale(frente_direita, DISTANCIA_SENSOR_IA / 2));

        int mapX_frente = sensor_frente_pos.x / FATOR_ESCALA;
        int mapY_frente = sensor_frente_pos.y / FATOR_ESCALA;
        int mapX_esq = sensor_esquerda_pos.x / FATOR_ESCALA;
        int mapY_esq = sensor_esquerda_pos.y / FATOR_ESCALA;
        int mapX_dir = sensor_direita_pos.x / FATOR_ESCALA;
        int mapY_dir = sensor_direita_pos.y / FATOR_ESCALA;

        bool parede_frente = (mapa[mapY_frente][mapX_frente] == 'P' || mapa[mapY_frente][mapX_frente] == 'p');
        bool parede_esq = (mapa[mapY_esq][mapX_esq] == 'P' || mapa[mapY_esq][mapX_esq] == 'p');
        bool parede_dir = (mapa[mapY_dir][mapX_dir] == 'P' || mapa[mapY_dir][mapX_dir] == 'p');

        if (parede_frente) inimigo->angulo += VELOCIDADE_ROTACAO * 5;
        else if (parede_dir) inimigo->angulo -= VELOCIDADE_ROTACAO;
        else if (parede_esq) inimigo->angulo += VELOCIDADE_ROTACAO;

        inimigo->velocidade = MeuVector2Add(inimigo->velocidade, MeuVector2Scale(frente, ACELERACAO * 0.95f));
    }

    inimigo->velocidade = MeuVector2Scale(inimigo->velocidade, 1.0f - ATRITO);
    if (MeuVector2Length(inimigo->velocidade) > VELOCIDADE_MAXIMA * 0.9f) {
        inimigo->velocidade = MeuVector2Scale(MeuVector2Normalize(inimigo->velocidade), VELOCIDADE_MAXIMA * 0.9f);
    }
    inimigo->posicao = MeuVector2Add(inimigo->posicao, inimigo->velocidade);
}


void AtualizarJogo(Carro *jogador, Carro inimigos[], CaixaItem caixas[], const char mapa[MAP_ALTURA][MAP_LARGURA + 1], Tiro tiros[], int numInimigos, EstadoJogo *estadoAtual, char *mensagemFim, int *feedbackTimer) {
    if (*feedbackTimer > 0) (*feedbackTimer)--;

    if (jogador->dano_timer > 0) jogador->dano_timer--;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) jogador->angulo -= VELOCIDADE_ROTACAO;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) jogador->angulo += VELOCIDADE_ROTACAO;

    Vector2 direcao = {cosf(jogador->angulo * DEG2RAD), sinf(jogador->angulo * DEG2RAD)};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) jogador->velocidade = MeuVector2Add(jogador->velocidade, MeuVector2Scale(direcao, ACELERACAO));
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) jogador->velocidade = MeuVector2Subtract(jogador->velocidade, MeuVector2Scale(direcao, ACELERACAO));

    if (IsKeyPressed(KEY_SPACE) && jogador->tem_arma) {
        jogador->tem_arma = false;
        bool tiroDisparado = false;
        for (int i = 0; i < MAX_TIROS && !tiroDisparado; i++) {
            if (!tiros[i].ativo) {
                tiros[i].ativo = true;
                tiros[i].posicao = jogador->posicao;
                tiros[i].velocidade = MeuVector2Scale(direcao, VELOCIDADE_MAXIMA * 2);
                tiroDisparado = true;
            }
        }
    }

    jogador->velocidade = MeuVector2Scale(jogador->velocidade, 1.0f - ATRITO);
    if (MeuVector2Length(jogador->velocidade) > VELOCIDADE_MAXIMA) jogador->velocidade = MeuVector2Scale(MeuVector2Normalize(jogador->velocidade), VELOCIDADE_MAXIMA);
    jogador->posicao = MeuVector2Add(jogador->posicao, jogador->velocidade);

    for (int i = 0; i < numInimigos; i++) {
        AtualizarInimigoIA(&inimigos[i], mapa);
    }

    for (int i = 0; i < MAX_TIROS; i++) {
        if (tiros[i].ativo) {
            tiros[i].posicao = MeuVector2Add(tiros[i].posicao, tiros[i].velocidade);
            if (tiros[i].posicao.x < 0 || tiros[i].posicao.x > TELA_LARGURA || tiros[i].posicao.y < 0 || tiros[i].posicao.y > GAME_AREA_ALTURA) tiros[i].ativo = false;

            bool tiroAcertou = false;
            for (int j = 0; j < numInimigos && !tiroAcertou; j++) {
                if (CheckCollisionPointCircle(inimigos[j].posicao, tiros[i].posicao, 10)) {
                    if (inimigos[j].tem_escudo) inimigos[j].tem_escudo = false;
                    else inimigos[j].vida -= DANO_TIRO;
                    inimigos[j].dano_timer = 15;
                    inimigos[j].velocidade = MeuVector2Zero();
                    tiros[i].ativo = false;
                    tiroAcertou = true;
                }
            }
        }
    }

    Carro* todosOsCarros[] = {jogador, &inimigos[0]};
    int totalCarros = 1 + numInimigos;

    for (int i = 0; i < totalCarros; i++) {
        Carro *carro = todosOsCarros[i];

        if (carro->posicao.x < 0) carro->posicao.x = 0;
        if (carro->posicao.x > GAME_AREA_LARGURA) carro->posicao.x = GAME_AREA_LARGURA;
        if (carro->posicao.y < 0) carro->posicao.y = 0;
        if (carro->posicao.y > GAME_AREA_ALTURA) carro->posicao.y = GAME_AREA_ALTURA;

        int mapX = (int)(carro->posicao.x / FATOR_ESCALA);
        int mapY = (int)(carro->posicao.y / FATOR_ESCALA);

        if (mapX >= 0 && mapX < MAP_LARGURA && mapY >= 0 && mapY < MAP_ALTURA) {
            char simboloMapa = mapa[mapY][mapX];
            if (simboloMapa == 'P' || simboloMapa == 'p') {
                carro->posicao = MeuVector2Subtract(carro->posicao, carro->velocidade);
                carro->velocidade = MeuVector2Zero();
                if(carro->vida > 0 && carro->dano_timer == 0) {
                    carro->vida -= DANO_PAREDE;
                    carro->dano_timer = 15;
                    if(i > 0) carro->ia_reversing_timer = 20;
                }
            }

            bool naChegada = (simboloMapa == 'L');
            if (naChegada && !carro->na_linha_de_chegada_anterior) {
                carro->voltas_completas++;
            }
            carro->na_linha_de_chegada_anterior = naChegada;

            if (i == 0) {
                Rectangle recCarro = {carro->posicao.x - 5, carro->posicao.y - 5, 10, 10};
                for (int j = 0; j < MAX_CAIXAS; j++) {
                    if (caixas[j].ativa && CheckCollisionRecs(recCarro, caixas[j].retangulo)) {
                        caixas[j].ativa = false;
                        switch(caixas[j].tipo) {
                            case ITEM_ARMA: carro->tem_arma = true; break;
                            case ITEM_ESCUDO: carro->tem_escudo = true; break;
                            case ITEM_BOMBA_REVES:
                                if(carro->tem_escudo) carro->tem_escudo = false;
                                else {
                                    carro->vida -= DANO_BOMBA_REVES;
                                    carro->dano_timer = 15;
                                }
                                break;
                        }
                        int randX, randY;
                        do {
                            randX = GetRandomValue(0, MAP_LARGURA - 1);
                            randY = GetRandomValue(0, MAP_ALTURA - 1);
                        } while (mapa[randY][randX] != ' ');
                        caixas[j].retangulo.x = (float)randX * FATOR_ESCALA;
                        caixas[j].retangulo.y = (float)randY * FATOR_ESCALA;
                        caixas[j].ativa = true;
                    }
                }
            }
        }
    }

    if (jogador->voltas_completas >= TOTAL_VOLTAS) {
        *estadoAtual = ESTADO_FIM;
        strcpy(mensagemFim, "VOCE VENCEU!");
    } else if (jogador->vida <= 0) {
        *estadoAtual = ESTADO_FIM;
        strcpy(mensagemFim, "GAME OVER");
    } else {
        bool inimigoVenceu = false;
        for (int i = 0; i < numInimigos && !inimigoVenceu; i++) {
            if (inimigos[i].voltas_completas >= TOTAL_VOLTAS) {
                *estadoAtual = ESTADO_FIM;
                strcpy(mensagemFim, "VOCE PERDEU!");
                inimigoVenceu = true;
            }
        }
    }
}

// Desenha todos os elementos visuais do jogo.
void DesenharJogo(const Carro *jogador, const Carro inimigos[], const CaixaItem caixas[], const char mapa[MAP_ALTURA][MAP_LARGURA + 1], int numInimigos, const Tiro tiros[], const char *feedbackMessage, int feedbackTimer) {
    DesenharMapa(mapa);

    for (int i = 0; i < MAX_CAIXAS; i++) {
        if(caixas[i].ativa) {
            DrawRectangleRec(caixas[i].retangulo, GOLD);
            DrawText("?", (int)caixas[i].retangulo.x + 2, (int)caixas[i].retangulo.y, 10, BLACK);
        }
    }

    Color corJogador = (jogador->dano_timer > 0 && (GetTime() * 10) - (int)(GetTime() * 10) > 0.5) ? WHITE : jogador->cor;
    Rectangle recJogador = {jogador->posicao.x, jogador->posicao.y, 20, 10};
    Vector2 origem = {10, 5};
    DrawRectanglePro(recJogador, origem, jogador->angulo, corJogador);
    if(jogador->tem_escudo) DrawCircleLines((int)jogador->posicao.x, (int)jogador->posicao.y, 15, SKYBLUE);

    for (int i = 0; i < numInimigos; i++) {
        Color corInimigo = (inimigos[i].dano_timer > 0 && (GetTime() * 10) - (int)(GetTime() * 10) > 0.5) ? WHITE : inimigos[i].cor;
        Rectangle recInimigo = {inimigos[i].posicao.x, inimigos[i].posicao.y, 20, 10};
        DrawRectanglePro(recInimigo, origem, inimigos[i].angulo, corInimigo);
    }

    for(int i = 0; i < MAX_TIROS; i++) {
        if(tiros[i].ativo) DrawCircleV(tiros[i].posicao, 3, YELLOW);
    }

    if (feedbackTimer > 0) {
        DrawRectangle(0, 20, TELA_LARGURA, 40, Fade(BLACK, 0.7f));
        DrawText(feedbackMessage, TELA_LARGURA/2 - MeasureText(feedbackMessage, 30)/2, 25, 30, GREEN);
    }

    DesenharHUD(jogador);
}

// Desenha a interface do utilizador .
void DesenharHUD(const Carro *jogador) {
    DrawRectangle(0, TELA_ALTURA - ALTURA_HUD, TELA_LARGURA, ALTURA_HUD, Fade(BLACK, 0.8f));
    DrawText(TextFormat("Vida: %d", jogador->vida), 20, TELA_ALTURA - ALTURA_HUD + 10, 20, (jogador->vida < 30) ? RED : WHITE);
    DrawText(TextFormat("Voltas: %d / %d", jogador->voltas_completas, TOTAL_VOLTAS), 150, TELA_ALTURA - ALTURA_HUD + 10, 20, WHITE);
    if (jogador->tem_arma) DrawText("ARMA PRONTA!", TELA_LARGURA - 200, TELA_ALTURA - ALTURA_HUD + 10, 20, GREEN);
}

// Desenha o menu principal.
void DesenharMenu() {
    int textX = (TELA_LARGURA - MeasureText("Rock N Roll Racing", 70)) / 2;
    DrawText("Rock N Roll Racing", textX, 80, 70, RED);

    textX = (TELA_LARGURA - MeasureText("(N) Novo Jogo", 40)) / 2;
    DrawText("(N) Novo Jogo", textX, 200, 40, WHITE);
    textX = (TELA_LARGURA - MeasureText("(X) Salvar Jogo", 40)) / 2;
    DrawText("(X) Salvar Jogo", textX, 300, 40, GRAY);
    textX = (TELA_LARGURA - MeasureText("(C) Carregar Jogo", 40)) / 2;
    DrawText("(C) Carregar Jogo", textX, 400, 40, WHITE);
    textX = (TELA_LARGURA - MeasureText("(Q) Sair do Jogo", 40)) / 2;
    DrawText("(Q) Sair do Jogo", textX, 500, 40, WHITE);
}

// Lida com a navegação do menu principal com base nas teclas pressionadas.
void AtualizarMenu(EstadoJogo *estadoAtual, bool *deveCarregar, bool *reiniciar) {
    if (IsKeyPressed(KEY_N)) {
        *estadoAtual = ESTADO_JOGO;
        *reiniciar = true;
        *deveCarregar = false;
    } else if (IsKeyPressed(KEY_C)) {
        *estadoAtual = ESTADO_JOGO;
        *reiniciar = true;
        *deveCarregar = true;
    } else if (IsKeyPressed(KEY_Q)) {
        *estadoAtual = ESTADO_CONFIRMA_SAIR;
    }
}

// Salva o estado atual do jogo num ficheiro binário.
void SalvarJogo(const JogoSalvo *jogo) {
    FILE* arquivo = fopen("corrida.sav", "wb");
    if (arquivo) {
        fwrite(jogo, sizeof(JogoSalvo), 1, arquivo);
        fclose(arquivo);
    } else {
        printf("ERRO: Nao foi possivel criar o ficheiro para salvar.\n");
    }
}

// Carrega o estado do jogo a partir de um ficheiro binário.
bool CarregarJogo(JogoSalvo *jogo) {
    FILE* arquivo = fopen("corrida.sav", "rb");
    if (arquivo) {
        fread(jogo, sizeof(JogoSalvo), 1, arquivo);
        fclose(arquivo);
        return true;
    }
    return false;
}
