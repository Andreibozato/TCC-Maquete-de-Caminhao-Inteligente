# Maquete de Caminhão Inteligente com Sensores: Manobrabilidade Guiada

Este repositório contém o código-fonte, esquemas de circuitos e documentação para o projeto de Conclusão de Curso (TCC) **Lita Avalon**. O objetivo do projeto é o desenvolvimento de uma maquete funcional de um caminhão semi-autônomo focado no auxílio à condução segura e manobrabilidade guiada, mitigando acidentes causados por falhas humanas.

---

## 👥 Integrantes do Grupo
* **Andrei Bozato Bandeira**
* **Felipe Gabriel Sousa Macedo**
* **Italo Bruno Silva**
* **Luiz Gustavo Neves do Valle**

**Orientadores:** Prof. José Antonio Meire e Prof. Luis Carlos da Silva  
**Instituição:** ETEC Lauro Gomes 
**Curso:** Ensino Médio com Habilitação Profissional de Técnico em Mecatrônica (M-Tec PI Mecatrônica)
**Ano:** 2025

---

## 📂 Estrutura de Pastas e Documentos

Abaixo está a descrição da organização do repositório e o significado de cada diretório e documento:

* **`/src` ou `/codigo_principal`**: Contém o arquivo fonte principal do firmware desenvolvido em C++ no ambiente Arduino IDE para o microcontrolador ESP32.
* **`/hardware` ou `/circuitos`**: Reúne os esquemas elétricos, cálculos de drivers de transistores (2N2222A) e mapeamento físico das conexões de alimentação e sensores na protoboard.
* **`/docs` ou `/monografia`**: Pasta reservada para a documentação textual do TCC (Monografia em PDF, apresentações e referências bibliográficas).
* **`README.md`**: Este arquivo de documentação com a visão geral e instruções de uso do projeto.

---

## 📝 Resumo do Projeto

O modelo **Lita Avalon** consiste em uma maquete física simulada de um caminhão elétrico controlado remotamente via Bluetooth. Utilizando o microcontrolador **ESP32** como unidade central de processamento, o sistema coleta dados de sensores de distância ultrassônicos e de um magnetômetro (bússola digital) para atuar em tempo real na assistência ao condutor, sem retirar deste a responsabilidade total da direção (Classificado nos níveis iniciais de automação SAE).

---

## 🛠️ Especificações Técnicas e Componentes

O desenvolvimento multidisciplinar integrou componentes eletrônicos, mecânicos e programação estruturada em C++:

* **Unidade Central (Cérebro):** ESP32 DEVKIT V1 (30 pinos, Dual-Core, Bluetooth clássico e BLE).
* **Atuador de Tração:** Motor DC 25GA370 (6V, 320 RPM nominal) acoplado a um motoredutor com relação de transmissão de 26:1, operando com engrenagens de polímero em configuração redutora de velocidade para ganho de torque.
* **Driver de Potência (Tração):** Módulo Ponte H L298N para inversão de sentido (Frente/Ré) e controle de velocidade por modulação de largura de pulso (PWM).
* **Atuador de Direção:** Micro Servo Motor SG90 (Controlado por ângulos de 0° a 180° via biblioteca `ESP32Servo`).
* **Sensores Base:** 
  * Sensor Ultrassônico HC-SR04 para leitura de distâncias.
  * Magnetômetro QMC5883L (Bússola I2C) para monitoramento de direção.
* **Interface Visual (Computador de Bordo):** Display OLED SSD1306 ($128\times64$ pixels) gerenciado com a biblioteca `Adafruit_SSD1306`.
* **Circuitos Auxiliares:** Transistores NPN 2N2222A atuando como drivers de corrente para os faróis dianteiros e setas (luzes indicadoras de direção).
* **Alimentação:** Duas baterias de lítio modelo 18650 (fornecendo ~8V em série) reguladas por um módulo Step Down MP1584EN ajustado para 5.5V para alimentar os periféricos.

---

## 🚘 Funções Semi-Autônomas Implementadas

O firmware integra rotinas lógicas para processar as informações periféricas e auxiliar na condução segura:

1. **Freio Automático:** Monitora a aproximação de obstáculos à frente e bloqueia a aceleração através do corte do ciclo ativo (duty cycle) do PWM caso atinja a distância crítica.
2. **Controle de Luz pela Distância:** Modula a intensidade luminosa dos faróis em LED (via PWM dedicado) baseado na proximidade de outros veículos.
3. **Assistência em Manobras de Viragem / Ponto Cego:** Executa rotinas de varredura com os sensores ultrassônicos traseiros em paralelo com o acionamento das setas para alertar sobre colisões laterais.
4. **Sistema Start Stop:** Desliga o ciclo ativo do motor e emite alertas visuais no computador de bordo para otimização de energia em paradas prolongadas.
5. **Alerta Sonoro:** Emissão de bips através de um Buzzer Ativo proporcional à proximidade de objetos durante a marcha ré ou frenagem de emergência.

---

## 💻 Configuração do Ambiente e Compilação

Para compilar o código contido na pasta `/src`:

1. Instale a **Arduino IDE** (Versão 2.3.5 ou superior).
2. Instale o driver **CP2102 da Silicon Labs** no sistema operacional para reconhecimento da porta COM do ESP32.
3. Nas *Preferências* da IDE, adicione a URL da placa adicional da Espressif:
   `https://dl.espressif.com/dl/package_esp32_index.json`
4. Instale as seguintes bibliotecas através do gerenciador interno:
   * `Adafruit SSD1306` e `Adafruit GFX Library`
   * `ESP32Servo`
   * `Ultrasonic`
5. Selecione a placa **DOIT ESP32 DEVKIT V1**, configure a porta serial correspondente e faça o upload.

## 📱 Aplicativo de Controle Avalon (Android)

O controle e a movimentação da maquete são realizados por meio de um aplicativo desenvolvido exclusivamente para o projeto, que se comunica com o ESP32 via protocolo **Bluetooth SPP (Serial Port Profile)**. 

### Principais Funcionalidades do App:
* **Controle de Movimentação:** Envio de comandos direcionais para frente, ré, esquerda e direita, traduzidos em sinais PWM para os motores.
* **Painel de Comando de Iluminação:** Botões dedicados para acionamento manual dos faróis de milha e das luzes de sinalização (setas).
* **Monitoramento e Telemetria:** O aplicativo funciona em via de mão dupla, recebendo dados enviados pelo caminhão e exibindo alertas de proximidade na tela do celular.
---
ETEC Lauro Gomes — Mecatrônica Industrial (M-Tec PI)
