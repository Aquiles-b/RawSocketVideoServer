# RawSocketVideoServer

O projeto de streaming de vídeo desenvolvido consiste em dois componentes principais: 
um cliente e um servidor, ambos conectados diretamente via cabo de rede, utilizando raw sockets
para comunicação. O cliente exibe uma lista de vídeos disponíveis no servidor, permitindo que o 
usuário selecione um vídeo para download e posterior reprodução. O vídeo é transferido integralmente 
para o cliente antes de ser exibido, evitando a reprodução progressiva típica de outros sistemas de streaming.

## Formato dos pacotes
![formato_pacote](https://github.com/user-attachments/assets/27f12260-e827-4190-891e-ced2b307e246)

O protocolo utilizado é inspirado no Kermit, com um formato de frame específico que inclui marcador
de início, tamanho, sequência, tipo, dados e CRC-8 para detecção de erros. O controle de fluxo é implementado 
através de uma janela deslizante usando o mecanismo go-back N, com até cinco mensagens sendo enviadas antes da 
necessidade de confirmação (ACK). Mensagens de controle, como solicitações e respostas, utilizam um modelo de "para-espera".

A implementação também trata timeouts, erros na comunicação, e utiliza diferentes tipos de mensagens,
como solicitações de lista, download de vídeo, confirmação (ack), negação (nack), entre outros. O sistema 
inclui robustez para lidar com possíveis falhas, garantindo que a transferência dos arquivos seja concluída com sucesso antes da exibição.
