# TCC_MonitorCardiacoOnline
Código Fonte do Monitor de Atividade Cardíaca Online

Funcionamento: 

O código baseia-se nas entradas LO+, LO- e SDN do monitor de condicionamento de potenciais bioelétricos, o AD8232. Ao receber o sinal de que os eletrodos estão conectados ao corpo do usuário, o sistema começa a realizar um teste de conexão. Caso seja bem sucedido, é exibida uma mensagem de sucesso e o IP associado à conexão local.

A partir da segunda etapa, qualquer alteração nos eletrodos que indique uma desconexão, aparecerá a mensagem "Leads off!", informando que algum dos eletrodos foi desconectado. Se não for o caso, o programa continua realizando as leituras indicadas no algoritmo. 
