// ============================================================
// Gabarito — Aula 09: Desafio do Protagonismo
// Estação de Telemetria: Temperatura, Luminosidade, Presença e Distância
// ============================================================

// --- 1. Declaração global dos pinos ---
const int PINO_LDR  = A0;   // Entrada analógica - luminosidade (LDR)
const int PINO_TEMP = A1;   // Entrada analógica - temperatura (LM35)
const int PINO_PIR  = 2;    // Entrada digital   - presença (PIR)
const int PINO_TRIG = 8;    // Saída digital     - disparo do ultrassônico
const int PINO_ECHO = 7;    // Entrada digital   - eco do ultrassônico

// --- 2. Constante de conversão ADC -> Celsius (LM35: 10mV/°C, ADC 10 bits) ---
const float BASE_CELSIUS = 0.488758;

// --- 3. Funções de condicionamento de sinal (transcritas da Etapa 2) ---

// Converte a leitura bruta do LM35 (0-1023) em graus Celsius
float condicionarTemperatura(int leituraBruta) {
  return (float)leituraBruta * BASE_CELSIUS;
}

// Converte a leitura bruta do LDR (0-1023) em percentual de luminosidade (0-100%)
int condicionarLuminosidade(int leituraBruta) {
  return (int)((leituraBruta / 1023.0) * 100.0);
}

// Dispara o pulso ultrassônico e retorna a distância em centímetros
long lerDistanciaUltrassonica() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);

  long duracaoDoPulso = pulseIn(PINO_ECHO, HIGH); // tempo de ida e volta do som (us)
  return duracaoDoPulso / 58;                     // converte tempo em distância (cm)
}

// --- 4. Configuração inicial ---
void setup() {
  Serial.begin(9600);
  pinMode(PINO_PIR, INPUT);
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  // PINO_LDR e PINO_TEMP são entradas analógicas e não exigem pinMode
}

// --- 5. Loop principal (fluxo de telemetria) ---
void loop() {
  // 5.1 Leitura bruta dos sensores, armazenada em variáveis locais
  int leituraLdrBruta  = analogRead(PINO_LDR);
  int leituraTempBruta = analogRead(PINO_TEMP);
  int presenca          = digitalRead(PINO_PIR);

  // 5.2 Condicionamento dos dados (a leitura bruta é passada como parâmetro,
  //     nunca impressa diretamente)
  int   luzPercentual = condicionarLuminosidade(leituraLdrBruta);
  float tempCelsius   = condicionarTemperatura(leituraTempBruta);
  long  distanciaCm   = lerDistanciaUltrassonica();

  // 5.3 Lógica condicional de segurança baseada no PIR
  String status;
  if (presenca == HIGH) {
    status = "ALERTA: Presenca Detectada";
  } else {
    status = "Area Segura";
  }

  // 5.4 Impressão da linha de telemetria formatada
  Serial.print("Temp: ");
  Serial.print(tempCelsius, 2);   // 2 casas decimais
  Serial.print(" C | Luz: ");
  Serial.print(luzPercentual);
  Serial.print("% | Dist: ");
  Serial.print(distanciaCm);
  Serial.print(" cm | Status: ");
  Serial.println(status);

  // 5.5 Intervalo de atualização do ciclo
  delay(1000);
}