#include <stdio.h>

// Constantes do sistema (definidas antes da compilacao)
#define LIMITE_TEMPERATURA 75.0
#define MOTOR_DESLIGADO 0
#define MOTOR_LIGADO 1

// Funcao simples que converte a leitura do sensor (0 a 1023) para porcentagem (0 a 100%)
int converterParaPorcentagem(int leituraSensor) {
    int porcentagem = (leituraSensor * 100) / 1023;
    return porcentagem;
}

int main() {
    // Variaveis de estado da estacao
    int estadoMotor = MOTOR_DESLIGADO;
    float temperaturaAtual = 25.0;
    int leituraBrutaSensor = 0;
    char opcaoMenu;

    do {
        // Exibicao do painel
        printf("\n================ PAINEL DE CONTROLE ================\n");
        printf("Status do Motor: %s\n", (estadoMotor == MOTOR_LIGADO) ? "LIGADO [OK]" : "DESLIGADO");
        printf("Temperatura:     %.1f C\n", temperaturaAtual);
        printf("Nivel do Sensor: %d%% (Leitura bruta: %d/1023)\n", 
               converterParaPorcentagem(leituraBrutaSensor), leituraBrutaSensor);
        
        // Alerta simples de seguranca
        if (temperaturaAtual >= LIMITE_TEMPERATURA) {
            printf(">>> ALERTA: TEMPERATURA CRITICA! SISTEMA EM RISCO! <<<\n");
        }
        printf("===================================================\n");

        // Menu de opcoes
        printf("1. Ligar Motor\n");
        printf("2. Desligar Motor\n");
        printf("3. Inserir Nova Temperatura\n");
        printf("4. Inserir Leitura do Sensor Analogico (0 a 1023)\n");
        printf("0. Encerrar Programa\n");
        printf("Escolha uma opcao: ");
        scanf(" %c", &opcaoMenu);

        switch (opcaoMenu) {
            case '1':
                // Trava de seguranca: nao permite ligar se estiver superaquecido
                if (temperaturaAtual >= LIMITE_TEMPERATURA) {
                    printf("\n[ERRO DE SEGURANCA] Impossivel ligar o motor: temperatura muito alta!\n");
                } else {
                    estadoMotor = MOTOR_LIGADO;
                    printf("\nMotor acionado com sucesso.\n");
                }
                break;

            case '2':
                estadoMotor = MOTOR_DESLIGADO;
                printf("\nMotor desligado com sucesso.\n");
                break;

            case '3':
                printf("\nDigite a nova temperatura em Celsius: ");
                scanf("%f", &temperaturaAtual);
                
                // Se superaquecer com motor ligado, desliga imediatamente
                if (temperaturaAtual >= LIMITE_TEMPERATURA && estadoMotor == MOTOR_LIGADO) {
                    estadoMotor = MOTOR_DESLIGADO;
                    printf("[PARADA DE EMERGENCIA] Motor desligado automaticamente por sobretemperatura!\n");
                }
                break;

            case '4':
                printf("\nDigite o valor da leitura do sensor (entre 0 e 1023): ");
                scanf("%d", &leituraBrutaSensor);
                
                if (leituraBrutaSensor < 0 || leituraBrutaSensor > 1023) {
                    printf("[AVISO] Leitura fora da faixa valida de 10 bits (0-1023). Ajustado para o limite.\n");
                    if (leituraBrutaSensor < 0) leituraBrutaSensor = 0;
                    if (leituraBrutaSensor > 1023) leituraBrutaSensor = 1023;
                }
                break;

            case '0':
                printf("\nEncerrando o simulador de controle. Ate logo!\n");
                break;

            default:
                printf("\nOpcao invalida! Digite apenas os numeros de 0 a 4.\n");
                break;
        }

    } while (opcaoMenu != '0');

    return 0;
}

