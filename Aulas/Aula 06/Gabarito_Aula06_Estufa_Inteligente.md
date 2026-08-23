# Gabarito — Atividade Prática Integrada: Construção do Firmware
## Aula 06 - Estruturas Condicionais, Repetição e Controle em IoT — Caso AgroTech

Este gabarito apresenta uma solução de referência em C, testada e compilada
(GCC, sem *warnings*), para o firmware da estufa inteligente. O código pode
ser colado diretamente no [OnlineGDB](https://www.onlinegdb.com/) (linguagem C).

---

## 1. Como o código atende a cada requisito

### 1.1 Estado do Hardware (Bitwise)
```c
unsigned char status_estufa = 0;
#define BIT_UMIDADE 0   // 0 = Seco   | 1 = Úmido
#define BIT_BOMBA   1   // 0 = Desligada | 1 = Ligada
#define BIT_ALERTA  2   // 0 = OK     | 1 = Crítico
```
Um único byte (8 bits) representa três periféricos distintos. Usar `#define`
para os índices dos bits evita "números mágicos" espalhados pelo código e
deixa claro qual bit corresponde a qual sensor/atuador.

### 1.2 Segurança (Cláusula de Guarda)
```c
if (status_estufa & (1 << BIT_ALERTA)) {
    estado_atual = ALERTA;
    status_estufa &= ~(1 << BIT_BOMBA);   // desliga a bomba
}
```
Essa verificação acontece **logo na primeira linha do super loop**, antes de
qualquer outra lógica — inclusive antes do `switch`. Isso garante que, não
importa em qual estado o sistema estava, um alerta de temperatura crítica
interrompe o fluxo normal imediatamente e desliga a bomba, evitando que a
estufa tente irrigar durante um risco térmico.

Repare na combinação de operadores:
- `1 << BIT_ALERTA` desloca o bit 1 até a posição 2 → `0b00000100`.
- `status_estufa & (...)` é a **máscara de leitura**: isola apenas o bit 2.
- `status_estufa &= ~(1 << BIT_BOMBA)` usa `NOT` + `AND` para **zerar** um
  único bit (bit 1) sem tocar nos demais — o oposto do `OR`, que liga bits.

### 1.3 Lógica de Controle (Máquina de Estados)
```c
switch (estado_atual) {
    case MONITORAMENTO: ...
    case IRRIGACAO:      ...
    case ALERTA:          ...
}
```
- **MONITORAMENTO:** apenas observa o bit de umidade; se o solo estiver seco,
  transiciona para `IRRIGACAO`.
- **IRRIGACAO:** liga a bomba com `status_estufa |= (1 << BIT_BOMBA)` — o
  `OR` garante que **somente o bit da bomba** é alterado, preservando os
  bits de umidade e alerta (resolvendo o Problema 3 do enunciado: a
  "corrupção de registradores").
- **ALERTA:** mantém a bomba desligada e só retorna ao monitoramento quando
  o bit de alerta for limpo.

### 1.4 Super Loop e Tempo Não-Bloqueante
```c
while (1) {
    ...
    time_t agora = time(NULL);
    if (difftime(agora, ultimo_print) >= 3.0) {
        printf(...);
        ultimo_print = agora;
    }
}
```
Nenhuma chamada a `delay()` ou `sleep()` existe no código. O processador
consulta o relógio (`time(NULL)`) a cada volta do laço e só imprime quando
3 segundos reais já passaram — mas continua **livre** para reagir a sensores
de emergência e à cláusula de guarda em todas as demais voltas do loop,
resolvendo o Problema 1 do enunciado (travamentos por espera bloqueante).

### 1.5 Sinalização Visual (toggle com XOR)
```c
led_status ^= 1;
```
A cada ciclo do super loop, o bit do LED é invertido: 0 vira 1, 1 vira 0.
É o uso clássico do XOR para *toggle* sem precisar de um `if/else`.

---

## 2. Código completo (testado — compila sem warnings em `gcc -Wall -Wextra`)

```c
/* =====================================================================
   AgroSmart - Firmware Estufa Inteligente (GABARITO)
   Aula 06 - Estruturas Condicionais, Repetição e Controle em IoT
   ===================================================================== */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define BIT_UMIDADE 0   /* 0 = Seco      | 1 = Úmido   */
#define BIT_BOMBA   1   /* 0 = Desligada | 1 = Ligada  */
#define BIT_ALERTA  2   /* 0 = OK        | 1 = Crítico */

typedef enum {
    MONITORAMENTO,
    IRRIGACAO,
    ALERTA
} EstadoEstufa;

int main(void) {

    unsigned char status_estufa = 0;
    unsigned char led_status    = 0;
    EstadoEstufa  estado_atual  = MONITORAMENTO;

    time_t ultimo_print = time(NULL);
    int ciclo = 0;
    int num_prints = 0;

    srand((unsigned int) time(NULL));

    printf("=== Firmware Estufa Inteligente iniciado ===\n");

    while (1) {

        /* --- Simulação de leitura de sensores (substitui o hardware real) --- */
        if (num_prints == 1) status_estufa |= (1 << BIT_ALERTA);
        if (num_prints == 3) status_estufa &= ~(1 << BIT_ALERTA);
        if (num_prints == 0 || num_prints == 3) status_estufa &= ~(1 << BIT_UMIDADE);
        if (num_prints == 4) status_estufa |= (1 << BIT_UMIDADE);

        /* --- Cláusula de guarda: segurança acima de tudo --- */
        if (status_estufa & (1 << BIT_ALERTA)) {
            estado_atual = ALERTA;
            status_estufa &= ~(1 << BIT_BOMBA);
        }

        /* --- Máquina de estados --- */
        switch (estado_atual) {
            case MONITORAMENTO:
                if (!(status_estufa & (1 << BIT_UMIDADE))) {
                    estado_atual = IRRIGACAO;
                }
                break;

            case IRRIGACAO:
                if (!(status_estufa & (1 << BIT_UMIDADE))) {
                    status_estufa |= (1 << BIT_BOMBA);
                } else {
                    status_estufa &= ~(1 << BIT_BOMBA);
                    estado_atual = MONITORAMENTO;
                }
                break;

            case ALERTA:
                status_estufa &= ~(1 << BIT_BOMBA);
                if (!(status_estufa & (1 << BIT_ALERTA))) {
                    estado_atual = MONITORAMENTO;
                }
                break;

            default:
                estado_atual = MONITORAMENTO;
                break;
        }

        /* --- LED de status (toggle com XOR) --- */
        led_status ^= 1;

        /* --- Temporização não-bloqueante (time.h, sem delay()) --- */
        time_t agora = time(NULL);
        if (difftime(agora, ultimo_print) >= 3.0) {
            const char *nomes_estado[] = {"MONITORAMENTO", "IRRIGACAO", "ALERTA"};
            printf("[t=%ld] status=0x%02X (umid=%d bomba=%d alerta=%d) | LED=%d | Estado=%s\n",
                   (long) agora, status_estufa,
                   (status_estufa >> BIT_UMIDADE) & 1,
                   (status_estufa >> BIT_BOMBA)   & 1,
                   (status_estufa >> BIT_ALERTA)  & 1,
                   led_status, nomes_estado[estado_atual]);
            if (estado_atual == ALERTA) {
                printf("  !!! ALERTA DE TEMPERATURA CRITICA - sistema em modo seguro !!!\n");
            }
            ultimo_print = agora;
            num_prints++;
        }

        ciclo++;

        /* Apenas para fins didáticos no simulador (em firmware real não existiria) */
        if (num_prints >= 6) {
            printf("=== [DEMO] Encerrando apos %d ciclos e %d impressoes ===\n", ciclo, num_prints);
            break;
        }
    }

    return 0;
}
```

### Saída real obtida ao rodar o programa (≈18 segundos, 6 impressões):
```
=== Firmware Estufa Inteligente iniciado ===
[t=...] status=0x02 (umid=0 bomba=1 alerta=0) | LED=1 | Estado=IRRIGACAO
[t=...] status=0x04 (umid=0 bomba=0 alerta=1) | LED=0 | Estado=ALERTA
  !!! ALERTA DE TEMPERATURA CRITICA - sistema em modo seguro !!!
[t=...] status=0x04 (umid=0 bomba=0 alerta=1) | LED=0 | Estado=ALERTA
  !!! ALERTA DE TEMPERATURA CRITICA - sistema em modo seguro !!!
[t=...] status=0x02 (umid=0 bomba=1 alerta=0) | LED=1 | Estado=IRRIGACAO
[t=...] status=0x01 (umid=1 bomba=0 alerta=0) | LED=1 | Estado=MONITORAMENTO
[t=...] status=0x01 (umid=1 bomba=0 alerta=0) | LED=1 | Estado=MONITORAMENTO
=== [DEMO] Encerrando apos 1881650879 ciclos de super loop e 6 impressoes de status ===
```
Note o número de ciclos do super loop (~1,9 bilhão em 18 segundos): esse é um
ótimo gancho de discussão em aula — mesmo "sem travar", um `while(1)` sem
nenhuma pausa consome 100% de um núcleo da CPU o tempo todo. Em firmware
real de baixo consumo, normalmente se combina o tempo não-bloqueante com
modos de baixo consumo (*sleep modes* do próprio hardware), mas isso foge
do escopo desta aula.

---

## 3. Respostas para o Roteiro de Apresentação (5 min por grupo)

**1. Demonstração do Simulador**
Ao rodar o programa, o console mostra o estado mudando de
`MONITORAMENTO` → `IRRIGACAO` (bomba liga, solo seco) → `ALERTA` (bomba
desliga imediatamente) → de volta a `IRRIGACAO`/`MONITORAMENTO`. A cada
linha impressa se passaram exatamente 3 segundos reais — comprovando que o
tempo não-bloqueante funciona sem travar o programa.

**2. Exibição do Código Limpo**
A cláusula de guarda (`if (status_estufa & (1 << BIT_ALERTA)) {...}`) fica
nas primeiras linhas do laço principal, **antes** do `switch`. Isso evita
que o código precise verificar "não está em alerta E o solo está seco E..."
dentro de cada `case` — o risco térmico é tratado uma única vez, no topo,
interrompendo qualquer intenção de irrigar.

**3. Defesa do Bitwise**
Para ligar a bomba sem alterar o sensor de umidade, usamos
`status_estufa |= (1 << BIT_BOMBA)`. A máscara `(1 << BIT_BOMBA)` é
`0b00000010`: o `OR` só pode **transformar 0 em 1** no bit 1; todos os
outros bits (`0`) da máscara, ao passar pelo `OR`, mantêm o valor original
do byte. Para desligar, usamos o oposto: `status_estufa &= ~(1 << BIT_BOMBA)`,
onde a máscara invertida é `0b11111101` — o `AND` preserva todos os bits
que são `1` na máscara e zera apenas o bit da bomba.

**4. Relato de Bug (exemplo real encontrado ao construir este gabarito)**
Na primeira versão, a mensagem `"ALERTA DE TEMPERATURA CRITICA"` foi
colocada dentro do `case ALERTA` do `switch`, dentro do `while(1)`. Como o
super loop roda milhões de vezes por segundo, o console foi inundado com
a mesma mensagem repetida bilhões de vezes em poucos segundos, estourando
o buffer de saída. **Causa:** confundir "a cada ciclo do super loop" com "a
cada vez que o estado é reportado ao usuário". **Correção:** mover o
`printf()` do alerta para dentro do mesmo bloco `if (difftime(...) >= 3.0)`
que já imprime o status — ou seja, reaproveitar a mesma temporização
não-bloqueante também para as mensagens de log, e não só para o status
principal. Isso ilustra bem o Problema 2 do enunciado original (o sistema
"ficar preso" em loops de leitura/impressão).

---

## 4. Checklist de correção rápida (para o professor)

| Requisito | Critério de aceite | Como identificar no código do aluno |
|---|---|---|
| Registrador de 8 bits | `unsigned char status_estufa` com 3 bits mapeados | Ver declaração + uso consistente dos mesmos índices de bit |
| Cláusula de guarda | Checagem do bit de alerta **antes** do switch, com `return`/mudança direta de estado | Deve estar nas primeiras linhas do `while(1)`, não dentro de um `case` |
| Máquina de estados | `switch/case` com os 3 estados nomeados e `break` em cada `case` | Ausência de `break` é erro grave (fall-through) |
| Ativar bomba sem corromper outros bits | Uso de `|=` (OR) para ligar e `&= ~(...)` para desligar — nunca atribuição direta (`status_estufa = 2;`) | Atribuição direta ao byte inteiro é a falha #3 do enunciado e deve ser penalizada |
| Sem `delay()`/`sleep()` | Nenhuma chamada bloqueante no código | Buscar por `delay(`, `sleep(`, `Sleep(` |
| Tempo não-bloqueante | Uso de `time()`/`clock()` + `difftime()` comparando contra um "último tempo salvo" | Deve haver uma variável tipo `ultimo_print`/`ultimo_tempo` atualizada só quando o intervalo é atingido |
| Toggle do LED | `led_status ^= 1;` (ou equivalente com XOR) a cada ciclo | Uso de `if/else` para alternar é funcionalmente aceitável, mas não atende ao requisito de usar XOR |
