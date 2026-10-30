# ESP8266 — CommonPlatform και firmware .ino

## OLED επιλογές για τις συγκεκριμένες οθόνες (128×64 I²C)

- **`OLED_TYPE 3` → CH1116 1,54″**, μέσω U8g2 `U8G2_SSD1309_128X64_NONAME0_F_HW_I2C` (η συμβατή ακολουθία αρχικοποίησης).
- **`OLED_TYPE 2` → SSD1309 1,3″**, με τον ίδιο οδηγό U8g2.
- **`OLED_TYPE 0` → χωρίς φυσική OLED**.

Η διάσταση δεν εγγυάται τον controller για όλα τα εμπορικά πάνελ: αυτή είναι η αντιστοίχιση των συγκεκριμένων συσκευών. Κάθε firmware δηλώνει διαφορετικό MQTT/OTA profile (`-ch1116`, `-ssd1309` ή `-none`).

- [CommonPlatform v1.05](./CommonPlatform/ESP8266_CommonPlatform_V1_05.ino).
- [EnergyMeter v7.01](./EnergyMeter/ESP8266_energymeter_v7.01.ino).
- [Ventilation v7.02](./Ventilation/ESP8266_ventilation_v7.02.ino) (προεπιλεγμένα χωρίς οθόνη).

**Οι υπάρχουσες προεπιλογές, εκδόσεις και μεταγλωττισμένα OTA BIN παραμένουν αμετάβλητα.** Η CH1116 επιλογή προστέθηκε στον πηγαίο κώδικα· δεν έχει επιβεβαιωθεί ως δοκιμασμένο firmware σε ESP8266.
