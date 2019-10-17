#include <ESP8266WiFi.h>
#include <Wire.h> // responsável pela comunicação com a interface i2c
#include <LiquidCrystal_I2C.h> // responsável pela comunicação com o display LCD
 
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

// Nome do seu wifi
//const char* ssid = "ifce-alunos"; 
// Senha do seu wifi
//const char* password = "ifce4lun0s";

// Nome do seu wifi
const char* ssid = "Nome da rede"; 
// Senha do seu wifi
const char* password = "Senha";

String request;
 
WiFiServer server(80); // Porto 80
WiFiServer sv(555);//Cria o objeto servidor na porta 555
WiFiClient cl;//Cria o objeto cliente.

void setup() {

    WiFi.mode(WIFI_AP);//Define o WiFi como Acess_Point.
    WiFi.softAP("NodeMCU", "");//Cria a rede de Acess_Point.

    sv.begin();//Inicia o servidor TCP na porta declarada no começo.

    //inicializa o display (16 colunas x 2 linhas)
    lcd.begin (16,2); // ou 20,4 se for o display 20x4
 
    Serial.begin(115200);
    delay(10);

    // Comunicação com a rede WiFi
    Serial.print("\n\nConnecting to "); // Mensagem apresentada no monitor série
    Serial.println(ssid); // Apresenta o nome da rede no monitor série

    WiFi.begin(ssid, password); // Inicia a ligação a rede
     
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("."); // Enquanto a ligação não for efectuada com sucesso é apresentado no monitor série uma sucessão de “.”
    }
    Serial.println("\nWiFi connected"); // Se a ligação é efectuada com sucesso apresenta esta mensagem no monitor série

    // Servidor
    server.begin(); // Comunicação com o servidor
    Serial.println("Servidor iniciado"); //é apresentado no monitor série que o  servidor foi iniciado

    // Impressão do endereço IP
    Serial.print("Use o seguinte URL para a comunicação: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP()); //Abrindo o Brower com este IP acedemos á pagina HTML de controlo dos LED´s, sendo que este IP só está disponível na rede à qual o ESP8266 se encontra ligado
    Serial.println("/");
}

void loop() {

    // Verificação se o cliente está conectado
    WiFiClient client = server.available();

    // Mostra a linha 1 do LCD
    lcd.setCursor(0,0);
    lcd.print(" --- LAESE ---  ");
    lcd.setCursor(0,1);
    lcd.print(" " + WiFi.localIP().toString() + "  ");

    // Aguarda a conexão de algum cliente
    if (!client) {

        // Wifi-Direct
      
        //Detecta se há clientes conectados no servidor.
        if (cl.connected()) {
            //Verifica se o cliente conectado tem dados para serem lidos.
            if (cl.available() > 0) {
                request = "";
                char z;
                while (cl.available() > 0) {
                    z = cl.read();
                    request += z;
                }
                request.replace(String(z),"");

                // MOstra mensagem recebida por TCP do Wifi-Direct
                mostraLCD();
    
                //Mostra a mensagem recebida do cliente no Serial Monitor.
                Serial.print("\nUm cliente enviou uma mensagem");
                Serial.print("\n...IP do cliente: ");
                Serial.print(cl.remoteIP());
                Serial.print("\n...IP do servidor: ");
                Serial.print(WiFi.softAPIP());
                Serial.print("\n...Mensagem do cliente: " + request + "\n");
    
                //Envia uma resposta para o cliente
                cl.print("\nO servidor recebeu sua mensagem");
                cl.print("\n...Seu IP: ");
                cl.print(cl.remoteIP());
                cl.print("\n...IP do Servidor: ");
                cl.print(WiFi.softAPIP());
                cl.print("\n...Sua mensagem: " + request + "\n");
            }
        } else {
            cl = sv.available();//Disponabiliza o servidor para o cliente se conectar.
            delay(1);
        }
        return;
    }

    // Espera até o cliente enviar dados
    Serial.println("novo cliente"); //Apresenta esta mensagem quando o cliente se liga ao servidor
    while(!client.available()){
        delay(1);
    }

    // Ler a primeira linha do pedido
    request = client.readStringUntil('\r');
    client.flush();

    // Mostra no LCD apenas se for HTTP de dados
    if (request.indexOf("/?texto") != -1) {
      mostraLCD();
    }

    // Montagem da Página WEB
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head> <meta charset=\"UTF-8\"/> </head>");
    client.print("<br><br><center>");
    client.print("<h1>LAESE</h1>");
    client.print("<h2>Transmissão de Mensagens por Conexão Cliente-Servidor e por Wifi-Direct com Protocolo TCP</h2> <br>");
    
    client.print("<h3>Digite seu texto: </h3>");
  
    client.println("<form method=\"get\">");
    client.println("<input type=\"text\" name=\"texto\" style=\"font-size:20px; text-align: center; width: 60%; height: 30px\" />  <br><br>");
    client.println("<button type=\"submit\" style=\"font-size:20px\">Enviar Texto</button><br />");
    client.println("</form>");

    client.print("</center><br><br>");
    client.println("</html>");
     
    delay(1);
     
    Serial.println("Cliente desconectado"); // Depois do cliente efectuar o pedido apresenta esta mensagem no monitor série
    Serial.println("");
}

void mostraLCD() {
  request.replace("GET /?texto=","");
  request.replace(" HTTP/1.1","");
  request.replace("+"," ");

  request = " Seu texto: " + request;
  
  int total = request.length() - 16;
  if(total < 0)
    total = 0;
  for(int i = 0; i <= total; i++){
    
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(request);

    delay(700);

    request.remove(0,1);
  }

  delay(1000);
  lcd.setCursor(0,1);
  lcd.print("                ");
}
