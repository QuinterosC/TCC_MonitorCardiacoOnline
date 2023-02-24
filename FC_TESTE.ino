#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/***************************************************************************************************************************************************************
                                                                MONITOR DE ATIVIDADE CARDÍACA ONLINE

FC -> Frequência Cardíaca
hr -> Variável publicada referente à FC
hrv -> Variabilidade de uma FC e a seguinte

***************************************************************************************************************************************************************/
//Constantes de Rede
#define WIFISSID "brisa-1154059" //ID da rede local
#define PASSWORD "foivyrps" //Senha da rede local
#define TOKEN "BBFF-DPIXiYN672iJ60HE3uiqP3HnRF4jYD" //Identificador de comunicação com o broker criado na plataforma da ubidots
#define MQTT_CLIENT_NAME "tccecg" //Nome do cliente

//Constantes do Broker
#define VARIABLE_LABEL_ECG "myecg"  //Variável criada na Ubidots para o ECG
#define VARIABLE_LABEL_FC "myheartrate"  //Variável criada na Ubidots para a FC
#define DEVICE_LABEL "esp8266"       //Broker criado na Ubidots

//Constantes ESP8266
#define SENSOR A0 //Entrada analógica da ESP8266

//Constantes da FC
#define shutdown_pin 12   //Pino SDN AD8232
#define threshold 215     //Limiar estabelecido para identificar a onda R
#define timer_value 10000   //10 segundos para cálculo da FC

long instance1=0, timer;
float hrv =0.0, hr = 72.0, interval = 0.0; //hr setado para 72 bpm iniciais, apenas para indicar na plataforma um valor padrão
float value = 0.0; 
int count = 0;  
bool flag = 0;

char mqttBroker[] = "industrial.api.ubidots.com"; //Endereço do broker, para saber onde enviar a informação na internet.

//Variáveis do ECG
char payload[100]; //Tamanho da mensagem armazenada.
char topic[150]; //Informações que serão publicadas em respectivos tópicos criados
char printBuffer[200];
//********************

//Variáveis da FC
char payload_FC[100]; //Tamanho da mensagem armazenada.
char topic_FC[150]; //Informações que serão publicadas em respectivos tópicos criados
char printBuffer_FC[200];
//********************

char str_sensor[10];      //Array para armazenar valores convertidos do ECG
char str_sensor_FC[10];  //Array para armazenar valores convertidos de HR

WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length){
    char p[length+1];
    memcpy(p, payload, length);
    p[length] = NULL;
    Serial.write(payload, length);
    Serial.println(topic);
}

//Função de reconexão, caso a conexão caia repentinamente
void reconnect(){
    while(!client.connected()){
        Serial.println("Tentando nova conexão MQTT");

        if(client.connect(MQTT_CLIENT_NAME, TOKEN, "")){
            Serial.println("Conectado!!!");
        }
        else{
            Serial.print("Conexão falhou, rc="); 
            Serial.print(client.state());
            Serial.print("Tentar novamente em 2 segundos");
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(14, INPUT); //Pino LO + responsável pela identificação da conexão do eletrodo
    pinMode(16, INPUT); //Pino LO- responsável pela identificação da conexão do eletrodo
    WiFi.begin(WIFISSID, PASSWORD); //Inicia a conexão com a rede local

    pinMode(SENSOR, INPUT); //Declara A0 como entrada de dados

    Serial.println();
    Serial.print("Aguardando por WiFi...");

    while(WiFi.status()!= WL_CONNECTED){
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("WiFi Conectado!!!");
    Serial.println("IP: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqttBroker,1883);
    client.setCallback(callback);
}

void loop(){ 
  //float myecg = analogRead(SENSOR);
  
  if(!client.connected()){
      reconnect();  
    }
    


    
  if((digitalRead(14) == 1)||(digitalRead(16) == 1)){ //Rotina para caso um dos eletrodos seja desconectado
    Serial.println("leads off!"); //Mensagem exibida, caso algum dos eletrodos seja desconectado

    digitalWrite(shutdown_pin, LOW); //Nível lógico baixo (modo standby)

    instance1 = micros();
    timer = millis();
  }

  else {
    digitalWrite(shutdown_pin, HIGH); //Nível lógico alto

    value = analogRead(SENSOR); //Leitura da entrada analógica
    value = map(value, 250, 400, 0, 100); //Estabelece os limites para plot do ECG

    //Limpa o conteúdo da vaiável tópico e indica onde deve ser salvo os dados do ECG
    sprintf(topic, "%s", ""); //Limpa o conteúdo de topic
    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    dtostrf(value, 4, 2, str_sensor);
    //dtostrf(hrv, 4, 1, str_sensor_hrv);

    //Início da impressão do ECG
    sprintf(payload, "%s", ""); //Limpa o payload
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_ECG); //Coloca o conteúdo de payload em VARIABLE_LABEL_ECG
    sprintf(payload, "%s {\"value\": %s", payload, str_sensor); //Exibe o valor lido do ECG
    sprintf(payload, "%s } }", payload);
    //Fim da impressão do ECG

    //Início da publicação do ECG
    sprintf(printBuffer, "%s", "");
    sprintf(printBuffer, "Publicando %s no topico %s",payload,topic); //Teste para saber se o dado está de fato sendo enviado
   // Serial.println(printBuffer);
    client.publish(topic, payload); //Publica o valor do pontos do ECG na variável cliente, inscrita no tópico "myecg"
    //Fim da publicação do ECG

    //Rotina para cálculo da FC
    if((value > threshold) && (!flag)) { //Rotina para quando a amplitude da onda identificada for menor que o limiar estabelecido
      count++;  //Contador para onda R. Incrementa toda vez que uma onda R é identificada  

      Serial.println("in"); //Mensagem enviada à serial, para confirmar a identificação da onda

      flag = 1; //Ativa a flag para que o programa não confunda as outras ondas com a onda R

      interval = micros() - instance1; //Salva o tempo entre uma onda R e a outra
      instance1 = micros(); //Salva o tempo da onda R anterior, para ser utilizado na próxima iteração
    }
    else if((value < threshold)) { //Rotina para quando o limiar estabelecido for maior que a amplitude identificada da onda
      flag = 0; //Baixa o nível da flag para a próxima iteração, de modo a identificar a próxima onda R
    }
    if ((millis() - timer) > timer_value) { //Rotina para quando o tempo contado for maior que 10 segundos
      hr = count*6.0; //Cálculo da FC
      //Limpa o conteúdo da variável tópico da FC e indica onde deve ser salvo os dados da FC
      sprintf(topic_FC, "%s", ""); //Limpa o conteúdo de topic FC
      sprintf(topic_FC, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

      dtostrf(hr, 4, 1, str_sensor_FC); //Converte o valor em float para string e salva na variável str_sensor_FC

      //Início da impressão da FC
      sprintf(payload_FC, "%s", ""); //Limpa o payload
      sprintf(payload_FC, "{\"%s\":", VARIABLE_LABEL_FC); //Coloca o conteúdo de payload2 em VARIABLE_LABEL_FC
      sprintf(payload_FC, "%s {\"value\": %s", payload_FC, str_sensor_FC); //Exibe o valor lido da FC no Broker
      sprintf(payload_FC, "%s } }", payload_FC);
      //Fim da impressão da FC

      //Início da publicação da FC
      sprintf(printBuffer_FC, "%s", "");
      sprintf(printBuffer_FC, "Publicando %s no topico %s",payload_FC,topic_FC);
     // Serial.println(printBuffer_FC);
      client.publish(topic_FC, payload_FC);
      //Fim da publicação da FC
      
      timer = millis();
      count = 0;  //Após a publicação da FC, zera o contador e prepara a variável para as próximas iterações 
    }
    hrv = hr/60.0 - interval/1000000; //Taxa de variabilidade entre um valor de FC e o próximo

    //Serial.print(hr);
    //Serial.print(",");
    //Serial.print(hrv);
    //Serial.print(",");
    //Serial.println(value);
    delay(50);
  }
}
