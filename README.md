# hms
Introducere

Home Monitoring System este un dispozitiv ce măsoară temperatura, umiditatea, cantitatea de luminădin casă și detectează mișcări. Se poate activa o alarmă pentru mișcare și se pot aprinde leduri desemnalizare. Controlul și vizualizarea datelor se realizează prin platforma Blynk, via WiFi.
Tot prin aplicația Blynk vom putea urmări grafice pentru a vedea cum a variat temperatura sauumiditatea din casa. Toată interacțiunea utilizatorului cu dispozitivul se va realiza printr-un dispozitivmobil.


Descriere generală

Proiectul folosește un NodeMCU cu ESP8266. Acesta este împărțit în 2 module:
Modulul central ce conține NodeMCU, LED-urile, Buzzerul și senzorul de temperatura/umiditate
Modulul auxiliar ce conține senzorul de mișcare și fotorezistorul utilizat pentru măsurareaintensității luminii
Sistemul extrage date de la senzorul de temperatură/umiditate, senzorul de mișcare și de lafotorezistor și le trimite către platforma Blynk de 10 ori pe secundă(conform recomandărilor Blynk IoT).Dacă sunt activate notificările de mișcare sau alarma din aplicație, atunci se vor transmite mesajelenecesare către utilizator și se va porni buzzer-ul/LED-ul albastru. Tot în cadrul aplicației vom avea șiLED-uri digitale ce vor arăta dacă senzorul PIR a detectat mișcare sau daca fotorezistorul înregistreazăo valoare peste un anumit prag setat in cod.
Alarma se declanșează dacă senzorul de mișcare are 2 activări consecutive în mai puțin de 15 secundepentru a evita rezultate fals-pozitive.
