# TCC_MonitorCardiacoOnline
Código Fonte do Monitor de Atividade Cardíaca Online

Funcionamento: 

O código baseia-se nas entradas LO+, LO- e SDN do monitor de condicionamento de potenciais bioelétricos, o AD8232. Ao receber o sinal de que os eletrodos estão conectados ao corpo do usuário, o sistema começa a realizar um teste de conexão. Caso seja bem sucedido, é exibida uma mensagem de sucesso e o IP associado à conexão local.

Em caso de desconexão da rede local ou ausência de rede, o programa tentará realizar uma nova conexão a cada 2 segundos.

A partir da segunda etapa, qualquer alteração nos eletrodos que indique uma desconexão, aparecerá a mensagem "Leads off!", informando que algum dos eletrodos foi desconectado. Se não for o caso, o programa continua realizando as leituras indicadas no algoritmo.

O programa conta com um contador, que auxilia no armazenamento das ondas R identificadas num período de 10 segundos. Posteriormente, esse contador será multiplicado por 6, onde será obtida a Frequência Cardíaca. Para isto, é realizada através de uma variável, que registra o limiar estabelecido sobre a amplitude da onda R, para que esta seja identificada e incrementada ao contador.

A obtenção do ECG se dá a partir da leitura da entrada analógica da ESP8266 conectada à saída (pino Output) do monitor de condicionamento, o AD8232. A forma de onda final pode ser visualizada na IDE do Arduino, através do Plotter Serial, onde o gráfico estará completamente parametrizado.
