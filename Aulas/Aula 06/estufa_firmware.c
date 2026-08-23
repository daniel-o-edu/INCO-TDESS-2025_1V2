/* =====================================================================
   AgroSmart - Firmware Estufa Inteligente (GABARITO)
   Aula 06 - Estruturas Condicionais, Repetição e Controle em IoT
   =====================================================================
   Requisitos atendidos:
   1. Registrador de 8 bits (status_estufa) via bitwise
   2. Clausula de guarda para Alerta de Temperatura
   3. Maquina de estados com switch/case
   4. Super Loop while(1) sem uso de delay() -> temporizacao com time.h
   5. LED de status alternado (toggle) com XOR
   ===================================================================== */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* ---------- Mapeamento dos bits do registrador status_estufa -------- */
#define BIT_UMIDADE 0   /* 0 = Seco   | 1 = Umido            */
#define BIT_BOMBA   1   /* 0 = Desligada | 1 = Ligada         */
#define BIT_ALERTA  2   /* 0 = OK     | 1 = Critico            */

/* ---------- Estados da maquina de estados --------------------------- */
typedef enum {
    MONITORAMENTO,
    IRRIGACAO,
    ALERTA
} EstadoEstufa;

int main(void) {

    unsigned char status_estufa = 0;   /* todos os bits zerados no boot   */
    unsigned char led_status    = 0;   /* LED de status (bit 0)           */
    EstadoEstufa  estado_atual  = MONITORAMENTO;

    time_t ultimo_print = time(NULL); /* marca de tempo do ultimo print  */
    int ciclo = 0;                    /* conta passagens pelo super loop  */
    int num_prints = 0;               /* conta quantas vezes ja imprimiu  */

    srand((unsigned int) time(NULL));

    printf("=== Firmware Estufa Inteligente iniciado ===\n");

    /* -------------------- SUPER LOOP (while(1)) ---------------------- */
    while (1) {

        /* ---- Simulacao de leitura de sensores (substitui hardware real) ----
           Os eventos abaixo sao amarrados ao numero de impressoes (que
           acontece a cada 3s "de verdade") e nao ao numero de voltas do
           laço, ja que o super loop roda milhoes de vezes por segundo.
           Isso existe apenas para tornar visivel, no console, o
           comportamento do firmware ao longo do tempo real. */
        if (num_prints == 1) {
            status_estufa |= (1 << BIT_ALERTA);      /* forca alerta critico apos o 1o print */
        }
        if (num_prints == 3) {
            status_estufa &= ~(1 << BIT_ALERTA);     /* tecnico resolve o alerta apos o 3o print */
        }
        if (num_prints == 0 || num_prints == 3) {
            status_estufa &= ~(1 << BIT_UMIDADE);    /* solo seco -> deve irrigar */
        }
        if (num_prints == 4) {
            status_estufa |= (1 << BIT_UMIDADE);     /* solo ficou umido -> para de irrigar */
        }

        /* ---------------- 2) CLAUSULA DE GUARDA (SEGURANCA) --------------
           Verificada logo no inicio do ciclo, ANTES de qualquer outra
           logica. Se a temperatura estiver critica, forca o estado de
           ALERTA e desliga a bomba imediatamente, sem passar pelo switch. */
        if (status_estufa & (1 << BIT_ALERTA)) {
            estado_atual = ALERTA;
            status_estufa &= ~(1 << BIT_BOMBA);      /* desliga bomba (AND com mascara invertida) */
        }

        /* ---------------- 3) MAQUINA DE ESTADOS (switch/case) ------------ */
        switch (estado_atual) {

            case MONITORAMENTO:
                /* Apenas coleta dados do solo; decide se deve irrigar */
                if (!(status_estufa & (1 << BIT_UMIDADE))) {
                    estado_atual = IRRIGACAO;
                }
                break;

            case IRRIGACAO:
                /* Ativa a bomba (bit 1) via OR, sem alterar os demais bits */
                if (!(status_estufa & (1 << BIT_UMIDADE))) {
                    status_estufa |= (1 << BIT_BOMBA);
                } else {
                    /* solo ja umido: desliga a bomba e volta a monitorar */
                    status_estufa &= ~(1 << BIT_BOMBA);
                    estado_atual = MONITORAMENTO;
                }
                break;

            case ALERTA:
                /* Mantem atuadores em modo seguro (a mensagem de alerta e'
                   exibida mais abaixo, junto com o print periodico de 3s -
                   NUNCA dentro do switch, pois o super loop roda milhoes de
                   vezes por segundo e um printf() aqui inundaria o console) */
                status_estufa &= ~(1 << BIT_BOMBA);
                /* so retorna ao monitoramento quando o alerta for limpo */
                if (!(status_estufa & (1 << BIT_ALERTA))) {
                    estado_atual = MONITORAMENTO;
                }
                break;

            default:
                estado_atual = MONITORAMENTO;
                break;
        }

        /* ---------------- 5) SINALIZACAO VISUAL (toggle com XOR) --------- */
        led_status ^= 1;

        /* ---------------- 4) TEMPO NAO-BLOQUEANTE (time.h) ---------------
           Em vez de delay(), consultamos o relogio a cada passagem pelo
           loop e so imprimimos quando 3 segundos tiverem passado. O
           processador nunca fica "cego": continua livre para reagir a
           novos eventos a qualquer momento. */
        time_t agora = time(NULL);
        if (difftime(agora, ultimo_print) >= 3.0) {
            const char *nomes_estado[] = {"MONITORAMENTO", "IRRIGACAO", "ALERTA"};
            printf("[t=%ld] status=0x%02X (umid=%d bomba=%d alerta=%d) | LED=%d | Estado=%s\n",
                   (long) agora,
                   status_estufa,
                   (status_estufa >> BIT_UMIDADE) & 1,
                   (status_estufa >> BIT_BOMBA)   & 1,
                   (status_estufa >> BIT_ALERTA)  & 1,
                   led_status,
                   nomes_estado[estado_atual]);
            if (estado_atual == ALERTA) {
                printf("  !!! ALERTA DE TEMPERATURA CRITICA - sistema em modo seguro !!!\n");
            }
            ultimo_print = agora;
            num_prints++;
        }

        ciclo++;

        /* ---------------------------------------------------------------
           OBS. PEDAGOGICA: em um microcontrolador real este while(1) roda
           para sempre, pois nao ha sistema operacional para "fechar" o
           programa. No simulador online (que roda em um PC comum), sem
           este limite o programa nunca terminaria. Por isso, apenas para
           fins didaticos, encerramos apos algumas impressoes de status
           (ou seja, apos alguns intervalos de 3s). Em producao real de
           firmware, esta condicao de saida NAO existiria.
           ------------------------------------------------------------- */
        if (num_prints >= 6) {
            printf("=== [DEMO] Encerrando simulacao apos %d ciclos de super loop e %d impressoes de status ===\n",
                   ciclo, num_prints);
            break;
        }
    }

    return 0;
}
