#ifndef __E436ADCF_E4E0_4C2D_97CA_A6300CC8E7A0__
#define __E436ADCF_E4E0_4C2D_97CA_A6300CC8E7A0__

#define FEATURE_BUZZER

#define BUZZER_GPIO 2
#define BUZZER_DAC 25

void init_buzzer() {
  // Initialize Buzzer
  ledcAttach(BUZZER_DAC, 2000, 8);
  pinMode(BUZZER_GPIO, OUTPUT);
  digitalWrite(BUZZER_GPIO, LOW);
}

void mute() { digitalWrite(BUZZER_GPIO, LOW); }
void unmute() { digitalWrite(BUZZER_GPIO, HIGH); }

void __play_canon_task(void *args) {
  note_t notes[] = {
      NOTE_Fs, NOTE_E,  NOTE_D,  NOTE_Cs, NOTE_B,  NOTE_A,  NOTE_B,  NOTE_Cs,
      NOTE_Fs, NOTE_E,  NOTE_D,  NOTE_Cs, NOTE_G,  NOTE_Fs, NOTE_G,  NOTE_Cs,
      NOTE_D,  NOTE_Cs, NOTE_D,  NOTE_D,  NOTE_Cs, NOTE_A,  NOTE_E,  NOTE_Fs,
      NOTE_D,  NOTE_D,  NOTE_Cs, NOTE_B,  NOTE_Cs, NOTE_Fs, NOTE_A,  NOTE_B,
      NOTE_B,  NOTE_Fs, NOTE_E,  NOTE_G,  NOTE_Fs, NOTE_E,  NOTE_D,  NOTE_Cs,
      NOTE_B,  NOTE_A,  NOTE_G,  NOTE_B,  NOTE_A,  NOTE_G,  NOTE_Fs, NOTE_G,
      NOTE_A,  NOTE_Fs, NOTE_G,  NOTE_A,  NOTE_Fs, NOTE_G,  NOTE_A,  NOTE_A,
      NOTE_B,  NOTE_Cs, NOTE_D,  NOTE_E,  NOTE_Fs, NOTE_G,  NOTE_Fs, NOTE_D,
      NOTE_E,  NOTE_Fs, NOTE_Fs, NOTE_G,  NOTE_A,  NOTE_B,  NOTE_A,  NOTE_G,
      NOTE_A,  NOTE_Fs, NOTE_G,  NOTE_A,  NOTE_G,  NOTE_B,  NOTE_A,  NOTE_G,
      NOTE_Fs, NOTE_E,  NOTE_Fs, NOTE_G,  NOTE_E,  NOTE_D,  NOTE_E,  NOTE_Fs,
      NOTE_G,  NOTE_A,  NOTE_B,  NOTE_G,  NOTE_B,  NOTE_A,  NOTE_B,  NOTE_Cs,
      NOTE_D,  NOTE_A,  NOTE_B,  NOTE_Cs, NOTE_D,  NOTE_E,  NOTE_Fs, NOTE_G,
      NOTE_A,  NOTE_A,  NOTE_Fs, NOTE_G,  NOTE_A,  NOTE_Fs, NOTE_E,  NOTE_Fs,
      NOTE_A,  NOTE_B,  NOTE_Cs, NOTE_D,  NOTE_E,  NOTE_Fs, NOTE_G,  NOTE_Fs,
      NOTE_D,  NOTE_Cs, NOTE_Fs, NOTE_D,  NOTE_E,  NOTE_Fs, NOTE_G,  NOTE_Fs,
      NOTE_D,  NOTE_Cs, NOTE_D,  NOTE_B,  NOTE_D,  NOTE_Cs, NOTE_B,  NOTE_A,
      NOTE_G,  NOTE_A,  NOTE_G,  NOTE_F,  NOTE_G,  NOTE_A,  NOTE_B,  NOTE_Cs,
      NOTE_D,  NOTE_B,  NOTE_D,  NOTE_Cs, NOTE_D,  NOTE_Cs, NOTE_D,  NOTE_E,
      NOTE_D,  NOTE_Cs, NOTE_D,  NOTE_B,  NOTE_Cs, NOTE_D};

  uint8_t octaves[] = {
      6, 6, 6, 6, 5, 5, 5, 6, 6, 6, 6, 6, 5, 5, 5, 6, 6, 6, 6, 5, 5, 5, 5,
      5, 5, 6, 6, 5, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 5, 6, 6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 6, 6, 6, 5, 6, 6, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 6, 6};

  int noteDurations[] = {
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  8,  8,
      8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
      8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  16, 16, 8,  16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 8,  16, 16, 8,  16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 8,  16, 16, 8,  16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 8,  16, 16, 8,  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 8,  16, 16,
      8,  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 8,  16, 16, 8,  16, 16, 16,
      16, 16, 16, 16, 16, 8,  16, 16, 8,  16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 8,  16, 16, 8,  16, 16, 16, 16, 16, 16, 16, 16, 8};

  Serial.println("Playing Canon in D");

  for (int thisNote = 48; thisNote < 158; thisNote++) {
    int noteDuration = 2000 / noteDurations[thisNote];
    unmute();
    ledcWriteNote(BUZZER_DAC, notes[thisNote], octaves[thisNote]);
    delay(noteDuration);
  }
  Serial.println("Canon in D done");
  vTaskDelete(NULL);
}

void play_canon() {
  xTaskCreatePinnedToCore(__play_canon_task, "Play Canon", 8192, NULL, 1, NULL,
                          ARDUINO_RUNNING_CORE);
}

void __play_beep_task(void *args) {
  Serial.println("Playing Beep");
  unmute();
  ledcWriteTone(BUZZER_DAC, 425);
  delay(500);
  mute();
  Serial.println("Beep done");
  vTaskDelete(NULL);
}

void play_beep() {
  // xTaskCreatePinnedToCore(__play_beep_task, "Play Beep", 2048, NULL, 1, NULL,
  //                         ARDUINO_RUNNING_CORE);

  Serial.println("Playing Beep");
  unmute();
  ledcWriteTone(BUZZER_DAC, 425);
  delay(500);
  mute();
  Serial.println("Beep done");
}

#endif /* __E436ADCF_E4E0_4C2D_97CA_A6300CC8E7A0__ */
