#include "wifi_save.h"

#define TOKEN "No_Token!"

WiFiServer server(80);
int client_count = 0;

int record_rst_time()
{
    int rst_time = 0;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open 打开NVS文件
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle my_handle;                                 
    err = nvs_open("storage", NVS_READWRITE, &my_handle); 
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("Restart counter = %d\n", restart_counter);
            rst_time = restart_counter;
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Write
        printf("Updating restart counter in NVS ... ");
        restart_counter++;
        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
    return rst_time;
}

void record_wifi(char *ssid, char *password, char *token)
{

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open 打开NVS文件
    printf("\n");
    printf("Opening Non-Volatile Wifi (NVS) handle... ");
    nvs_handle my_handle;                              
    err = nvs_open("Wifi", NVS_READWRITE, &my_handle); 
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");
        
        // Write
        printf("Updating ssid in NVS ... ");
        err = nvs_set_str(my_handle, "ssid", ssid);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        printf("Updating password in NVS ... ");
        err = nvs_set_str(my_handle, "password", password);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        printf("Updating token in NVS ... ");
        err = nvs_set_str(my_handle, "token", token);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
}

//检测ssid名称
void check_wifi(char *ssid, char *password, char *token)
{
    char saved_ssid[SSID_LENGTH];
    size_t ssid_length = SSID_LENGTH;
    char saved_password[SSID_LENGTH];
    size_t password_length = SSID_LENGTH;
    char saved_token[SSID_LENGTH];
    size_t token_length = SSID_LENGTH;
    // 初始化NVS，并检查初始化情况
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 如果NVS分区被占用则对其进行擦除
        // 并再次初始化
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open 打开NVS文件
    printf("\n");
    printf("Opening Non-Volatile Wifi (NVS) handle... \n");
    nvs_handle my_handle;                              // 定义不透明句柄
    err = nvs_open("Wifi", NVS_READWRITE, &my_handle); // 打开文件
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Read
        printf("Reading ssid and password from NVS ... \n");

        err = nvs_get_str(my_handle, "ssid", saved_ssid, &ssid_length);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("ssid: %s\n", saved_ssid);
            printf("ssid length= %d\n", ssid_length);
            strcpy(ssid, saved_ssid);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        err = nvs_get_str(my_handle, "password", saved_password, &password_length);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("password: %s\n", saved_password);
            printf("password length= %d\n", password_length);
            strcpy(password, saved_password);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        err = nvs_get_str(my_handle, "token", saved_token, &token_length);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("token: %s\n", saved_token);
            printf("token length= %d\n", token_length);
            strcpy(token, saved_token);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
    return;
}

void ap_init()
{
    //WiFi.softAP(ssid, password);
    WiFi.softAP("Nodo IoT I+D");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();
}

int wifi_config_server()
{

    WiFiClient client = server.available(); // listen for incoming clients

    if (client) // if you get a client,
    {
        Serial.println("---------------------------------------------------");
        Serial.printf("Index:%d\n", client_count);
        client_count++;
        Serial.println("New Client."); // print a message out the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        // the content of the HTTP response follows the header:
                        client.print("<img src=data:image/jpeg;base64,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>");
                        client.print("<h1>NODO IOT</h1><br><h2> WIFI CONFIG</h2><br>");
                        client.print(" <a href=\"/wifi_set\">Configurar WIFI</a><br>");
                       
                                     // The HTTP response ends with another blank line:
                        client.println();
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                         client.println("<style>html { font-family: Helvetica; display: inline-block;color:#000000; background-color: #A6A6A6; margin: 0px auto; text-align: center;}");
                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
                //show wifiset page
                if (currentLine.endsWith("GET /wifi_set"))
                {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print("<img src=data:image/jpeg;base64,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>");
                    client.print("<h1>NODO IOT</h1><br><h2> WIFI CONFIG</h2><br>");
                    client.print("<form action=\"/set_over\">SSID:<br><input type=\"text\" name=\"ssid\"><br>PASSWORD:<br><input type=\"text\" name=\"password\"><br><br>TOKEN:<br><input type=\"text\" name=\"[token\"><br><br>");
                    client.print("<input type=\"submit\" value=\"Set\"></form>");
                    // The HTTP response ends with another blank line:
                    client.println();

                    client.println("<!DOCTYPE html><html>");
                    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                    client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
                    client.println("<style>html { font-family: Helvetica; display: inline-block;color:#000000; background-color: #A6A6A6; margin: 0px auto; text-align: center;}");
   
                    // break out of the while loop:
                    break;
                }

                if (currentLine.endsWith("GET /set_over"))
                {
                    String get_request = "";
                    //read GET next line
                    while (1)
                    {
                        char c_get = client.read();
                        Serial.write(c_get);
                        if (c_get == '\n')
                        {
                            break;
                        }
                        else
                        {
                            get_request += c_get;
                        }
                    }

                    //set_wifi_from_url(server.uri());
                    set_wifi_from_url(get_request);

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print("<h1>CleanLight</h1><br><h2>Nodo IoT WIFI</h2><br>");
                    client.print("Set Successful<br>");
                    client.println("<style>html { font-family: Helvetica; display: inline-block;color:#000000; background-color: #a6a6a6; margin: 0px auto; text-align: center;}");
                    client.println();

                    client.stop();
                    Serial.println("Client Disconnected.");

                    return 0;
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
    return 1;
}

void set_wifi_from_url(String get_url)
{
    //get_url = "http://192.168.4.1/set_over?ssid=Makerfabs&password=20160704&[token=abcdetoken"
    int str_len = 0;
    int ssid_add = 0;
    int tkn_add =0;
    int pwd_add = 0;
    int end_add = 0;

    String ssid = "";
    String pwd = "";
    String tkn = "";

    str_len = get_url.length();
    ssid_add = get_url.indexOf('?');
    pwd_add = get_url.indexOf('&');
    tkn_add = get_url.indexOf('%');
    end_add = get_url.indexOf(' ');

    ssid = get_url.substring(ssid_add + 6, pwd_add);
    pwd = get_url.substring(pwd_add + 10, tkn_add-1);
    tkn = get_url.substring(tkn_add + 9, end_add);


    Serial.println();
    Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
    Serial.println("Get ssid and password from url:");
    Serial.println(get_url);
    Serial.println(ssid);
    Serial.println(pwd);
    Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");

    record_wifi((char *)ssid.c_str(), (char *)pwd.c_str(),(char *)tkn.c_str());
}

// int wifi_set_main()
// {
//     char ssid[SSID_LENGTH];
//     char password[SSID_LENGTH];
//     pinMode(WIFI_SET_PIN, INPUT_PULLUP);

//     check_wifi(ssid, password);
//     if (strcmp(ssid, "NULL") == 0 )
//     {
//         //检查SSID是否为NULL
//         Serial.println("Check SSID is NULL,please connect \"Makerfabs_ap\".");
//         Serial.println("And visit 192.168.4.1 to set WIFI.");
//         ap_init();
//         while (wifi_config_server())
//             ;

//         //设置完成后休眠3秒重启
//         delay(3000);
//         esp_restart();
//     }
//     else
//     {
//         //3秒内拉低WIFI_SET_PIN则恢复出场设置并重启
//         Serial.println("Check WIFI_SET_PIN");
//         int runtime = millis();
//         int starttime = runtime;
//         while ((runtime - starttime) < 3000)
//         {
//             if (digitalRead(WIFI_SET_PIN) == LOW)
//             {
//                 Serial.println("Init WIFI to default \"NULL\"");
//                 record_wifi("NULL", "NULL0000");
//                 delay(1000);
//                 esp_restart();
//             }
//             Serial.print(".");
//             delay(100);
//             runtime = millis();
//         }
//         Serial.println();

//         //Connect wifi
//         Serial.println("Connecting WIFI");
//         WiFi.begin(ssid, password);

//         int connect_count = 0;

//         //10S未连接上自动跳过并返回0
//         while (WiFi.status() != WL_CONNECTED)
//         {
//             delay(500);
//             Serial.print(".");
//             connect_count++;
//             if(connect_count > 10)
//                 return 0;
//         }

//         // 成功连接上WIFI则输出IP并返回1
//         Serial.println("");
//         Serial.println("WiFi connected");
//         Serial.println("IP address: ");
//         Serial.println(WiFi.localIP());

//         return 1;
//     }
// }

int wifi_set_main()
{
    char ssid[SSID_LENGTH];
    char password[SSID_LENGTH];
    char token[SSID_LENGTH];
    pinMode(WIFI_SET_PIN, INPUT_PULLUP);

    check_wifi(ssid, password,token);

    Serial.println("Check WIFI_SET_PIN");
    int runtime = millis();
    int starttime = runtime;
    while ((runtime - starttime) < 3000)
    {
        if (digitalRead(WIFI_SET_PIN) == LOW)
        {

            Serial.println("connect \"I+D Solbox\".");
            Serial.println("192.168.4.1 to set WIFI.");
            ap_init();
            while (wifi_config_server())
                ;
            delay(3000);
            esp_restart();
            return 0;
        }
        Serial.print(".");
        delay(100);
        runtime = millis();
    }
    Serial.println();

    //Connect wifi
    Serial.println("Connecting WIFI");
    WiFi.begin(ssid, password);

    int connect_count = 0;

    //10S未连接上自动跳过并返回0
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        connect_count++;
        if (connect_count > 10)
            return 0;
    }

    // 成功连接上WIFI则输出IP并返回1
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    return 1;
}

void nvs_test()
{
    char ssid[SSID_LENGTH] = "";
    char password[SSID_LENGTH] = "";
    char token[SSID_LENGTH] = "";
    int rst_time = 0;

    check_wifi(ssid, password, token);
    rst_time = record_rst_time();

    sprintf(ssid, "ssid_%d", rst_time);
    sprintf(password, "password_%d", rst_time);

    record_wifi(ssid, password, token);

    // Restart module
    for (int i = 10; i >= 0; i--)
    {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}


// Display the HTML web page
