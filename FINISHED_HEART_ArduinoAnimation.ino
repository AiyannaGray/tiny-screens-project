#include "Arduino_LED_Matrix.h"

#define TRIG A0
#define ECHO A1

ArduinoLEDMatrix matrix;

long duration;
int distanceCm = 0;

unsigned long lastSensorRead = 0;
unsigned long lastBeatEvent = 0;

int beatInterval = 900;
int beatStage = 0;
int heartState = 0;   // 0=tiny, 1=medium, 2=giant


// TINY rest
byte tinyRest[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,1,0,0,0,1,0,0,0,0},
  {0,0,1,1,1,0,1,1,1,0,0,0},
  {0,0,0,1,1,1,1,1,0,0,0,0},
  {0,0,0,0,1,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

// TINY beat
byte tinyBeat[8][12] = {
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,1,1,1,1,1,1,0,0,0},
  {0,0,1,1,1,1,1,1,1,1,0,0},
  {0,0,0,1,1,1,1,1,1,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

// MEDIUM rest
byte mediumRest[8][12] = {
  {0,0,0,1,1,0,0,1,1,0,0,0},
  {0,0,1,1,1,1,1,1,1,1,0,0},
  {0,1,1,1,1,1,1,1,1,1,1,0},
  {0,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,1,1,1,1,0,0},
  {0,0,0,1,1,1,1,1,1,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0}
};

// MEDIUM beat
byte mediumBeat[8][12] = {
  {0,0,1,1,1,0,0,1,1,1,0,0},
  {0,1,1,1,1,1,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,1,1,1,1,0,0},
  {0,0,0,1,1,1,1,1,1,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0}
};

// GIANT rest
byte giantRest[8][12] = {
  {0,1,1,1,1,0,0,1,1,1,1,0},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,1,1,1,1,0,0},
  {0,0,0,1,1,1,1,1,1,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0}
};

// GIANT beat
byte giantBeat[8][12] = {
  {1,1,1,1,1,0,0,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,1,1,1,1,0,0}
};

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  matrix.begin();
  matrix.renderBitmap(tinyRest, 8, 12);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= 80) {
    distanceCm = readDistance();

    if (distanceCm <= 0 || distanceCm > 200) {
      distanceCm = 120;
    }

    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.print(" cm   ");

    if (distanceCm > 60) {
      heartState = 0;
      beatInterval = 1100;
      Serial.println("tiny / slow");
    }
    else if (distanceCm > 20) {
      heartState = 1;
      beatInterval = 700;
      Serial.println("medium / normal");
    }
    else {
      heartState = 2;
      beatInterval = 300;
      Serial.println("giant / fast");
    }

    lastSensorRead = now;
  }

  updateHeartbeat(now);
}

int readDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  return duration * 0.034 / 2;
}

void updateHeartbeat(unsigned long now) {
  switch (beatStage) {
    case 0:
      if (now - lastBeatEvent >= (unsigned long)beatInterval) {
        drawHeart(true);   
        lastBeatEvent = now;
        beatStage = 1;
      } else {
        drawHeart(false);  
      }
      break;

    case 1:
      if (now - lastBeatEvent >= 90) {
        drawHeart(false);
        lastBeatEvent = now;
        beatStage = 2;
      }
      break;

    case 2:
      if (now - lastBeatEvent >= 110) {
        drawHeart(true);  
        lastBeatEvent = now;
        beatStage = 3;
      }
      break;

    case 3:
      if (now - lastBeatEvent >= 70) {
        drawHeart(false);
        lastBeatEvent = now;
        beatStage = 0;
      }
      break;
  }
}

void drawHeart(bool expanded) {
  if (heartState == 0) {
    if (expanded) matrix.renderBitmap(tinyBeat, 8, 12);
    else          matrix.renderBitmap(tinyRest, 8, 12);
  }
  else if (heartState == 1) {
    if (expanded) matrix.renderBitmap(mediumBeat, 8, 12);
    else          matrix.renderBitmap(mediumRest, 8, 12);
  }
  else {
    if (expanded) matrix.renderBitmap(giantBeat, 8, 12);
    else          matrix.renderBitmap(giantRest, 8, 12);
  }
}
