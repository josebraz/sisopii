## Trabalho de comunicação entre processos distribuídos

Faz parte da cadeira de Sistemas Operacionais II na UFRGS. Consiste basicamente em desenvolver um twitter em modo texto.

### Prerequisitos (levantados da especificação)

#### Servidor
* Persistência dos usuários - OK
* Carregar a persistência quando iniciar o servidor
* Abrir um servidor UDP - OK
* Gerenciador de sessões de usuários (iniciar sessão, máximo duas por usuário)
* Fazer o dispatcher das notificações (enviar para todos os seguidores atuais do usuário)
* Enviar as notificações pendentes desde a última sessão iniciada (cuidar caso com duas sessões ativas)

#### Cliente
* Abrir um cliente UDP - OK
* Parametros de inicialização - OK
* Iniciar uma sessão no servidor (informando usuário)
* Seguir um outro usuário (para receber notificações dele)
* Possibilidade de postar (enviar notificação)

### Implementação
* Organizar em uma estrutura de módulos - OK
* Funções de marshalling e unmarshalling - OK
* Lógica de esperar pela resposta do servidor após enviar mensagem - OK
* Para cada cliente, ter uma instância de um Produtor-Consumidor de notificações em que:
    * Produzir: Receber a notificação do enviada do cliente, atualizar a 
    lista de notificação pendentes de envio, para cada servidor atualizar 
    a fila de notificações pendentes de envio.
    * Consumir: recuperar (e remover) um identificador de notificação da 
    fila de pendentes, enviar a respectiva notificação ao processo cliente 
    e decrementar o valor de envios pendentes no metadado da notificação

### Definições
* Usuário: Cadeia de 4 a 20 caracteres (ex: @estudante)
* Notificação: Mensagens de texto curtas atreladas a um usuário