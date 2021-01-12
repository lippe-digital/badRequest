# badRequest

Ein Aruduino Sketch basierend auf dem BearSLL_CertStore Example:
https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/BearSSL_CertStore/BearSSL_CertStore.ino

Der ursprüngliche GET request wurde durch einen POST request ersetzt:
  
  ```
  Serial.printf("Connected!\n-------\n");
  client->write("POST ");
  client->write(path);
  client->write(" HTTP/1.0\r\nHost: ");
  client->write(host);
  client->write("\r\n");
  client->write("Content-Type: application/json\r\n");
  client->write("{\"message\": \"**Backup** was successfully finished.\",\"priority\": 5,\"title\": \"Backup\"}\r\n");
  client->write("\r\n");
  ```
  
  Der POST request erzeugt einen 404 response:
  
  ```
HTTP/1.1 400 Bad Request
Date: Tue, 12 Jan 2021 10:50:27 GMT
Content-Type: application/json
Content-Length: 64
Connection: close

{"error":"Bad Request","errorCode":400,"errorDescription":"EOF"}
 ``` 
  
