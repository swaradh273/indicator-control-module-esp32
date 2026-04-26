// Pin mapping
const int pb1 = 19;   // Right button
const int pb2 = 22;   // Left button
const int led1 = 23;  // Right LED
const int led2 = 18;  // Left LED

const unsigned long PRESS_TIME = 1000;   // 1 second
const unsigned long BLINK_TIME = 300;    // 300 ms

bool rightIndicator = false;
bool leftIndicator = false;
bool hazardMode = false;

unsigned long rightPressStart = 0;
unsigned long leftPressStart = 0;
unsigned long bothPressStart = 0;

bool rightHandled = false;
bool leftHandled = false;
bool bothHandled = false;

unsigned long hazardExitRightStart = 0;
unsigned long hazardExitLeftStart = 0;
bool hazardExitRightHandled = false;
bool hazardExitLeftHandled = false;

unsigned long lastBlinkTime = 0;
bool blinkState = false;

void setup() {
  pinMode(pb1, INPUT_PULLUP);
  pinMode(pb2, INPUT_PULLUP);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  Serial.begin(9600);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  Serial.println("===== SYSTEM STARTED =====");
  Serial.println("PB1=RIGHT(GPIO19), PB2=LEFT(GPIO22)");
  Serial.println("LED1=RIGHT(GPIO23), LED2=LEFT(GPIO18)");
}

void loop() {
  unsigned long now = millis();

  bool btnRightPressed = (digitalRead(pb1) == LOW);
  bool btnLeftPressed  = (digitalRead(pb2) == LOW);

  // 🔹 INPUT STATE PRINT
  Serial.print("[INPUT] Right=");
  Serial.print(btnRightPressed ? "PRESSED" : "RELEASED");
  Serial.print(" | Left=");
  Serial.println(btnLeftPressed ? "PRESSED" : "RELEASED");

  // Blink timing
  if (now - lastBlinkTime >= BLINK_TIME) {
    lastBlinkTime = now;
    blinkState = !blinkState;

    Serial.print("[BLINK] State=");
    Serial.println(blinkState ? "ON" : "OFF");
  }

  // ---------- BOTH BUTTONS PRESSED -> ENTER HAZARD ----------
  if (btnRightPressed && btnLeftPressed) {
    if (bothPressStart == 0) {
      bothPressStart = now;
      Serial.println("[EVENT] Both buttons pressed → Starting hazard timer");
    }

    if (!bothHandled && (now - bothPressStart >= PRESS_TIME)) {
      hazardMode = true;
      rightIndicator = false;
      leftIndicator = false;
      bothHandled = true;

      Serial.println("[MODE CHANGE] >>> HAZARD MODE ENABLED <<<");
    }
  } else {
    bothPressStart = 0;
    bothHandled = false;
  }

  // ---------- HAZARD MODE ----------
  if (hazardMode) {
    Serial.println("[MODE] Hazard active...");
    handleHazardExit(btnRightPressed, btnLeftPressed, now);
    updateLEDs();
    return;
  }

  // ---------- RIGHT BUTTON ----------
  if (btnRightPressed && !btnLeftPressed) {
    if (rightPressStart == 0) {
      rightPressStart = now;
      Serial.println("[EVENT] Right button pressed → Timer started");
    }

    if (!rightHandled && (now - rightPressStart >= PRESS_TIME)) {
      rightHandled = true;

      Serial.println("[ACTION] Right held for 1 sec");

      if (leftIndicator) {
        leftIndicator = false;
        rightIndicator = true;
        Serial.println("[SWITCH] LEFT OFF → RIGHT ON");
      } else {
        rightIndicator = !rightIndicator;
        Serial.print("[TOGGLE] RIGHT=");
        Serial.println(rightIndicator ? "ON" : "OFF");
      }
    }
  } else {
    rightPressStart = 0;
    rightHandled = false;
  }

  // ---------- LEFT BUTTON ----------
  if (btnLeftPressed && !btnRightPressed) {
    if (leftPressStart == 0) {
      leftPressStart = now;
      Serial.println("[EVENT] Left button pressed → Timer started");
    }

    if (!leftHandled && (now - leftPressStart >= PRESS_TIME)) {
      leftHandled = true;

      Serial.println("[ACTION] Left held for 1 sec");

      if (rightIndicator) {
        rightIndicator = false;
        leftIndicator = true;
        Serial.println("[SWITCH] RIGHT OFF → LEFT ON");
      } else {
        leftIndicator = !leftIndicator;
        Serial.print("[TOGGLE] LEFT=");
        Serial.println(leftIndicator ? "ON" : "OFF");
      }
    }
  } else {
    leftPressStart = 0;
    leftHandled = false;
  }

  updateLEDs();
}

void handleHazardExit(bool btnRightPressed, bool btnLeftPressed, unsigned long now) {
  if (btnRightPressed && !btnLeftPressed) {
    if (hazardExitRightStart == 0) {
      hazardExitRightStart = now;
      Serial.println("[HAZARD EXIT] Right button pressed");
    }

    if (!hazardExitRightHandled && (now - hazardExitRightStart >= PRESS_TIME)) {
      hazardMode = false;
      hazardExitRightHandled = true;
      Serial.println("[MODE CHANGE] >>> HAZARD OFF (RIGHT) <<<");
    }
  } else {
    hazardExitRightStart = 0;
    hazardExitRightHandled = false;
  }

  if (btnLeftPressed && !btnRightPressed) {
    if (hazardExitLeftStart == 0) {
      hazardExitLeftStart = now;
      Serial.println("[HAZARD EXIT] Left button pressed");
    }

    if (!hazardExitLeftHandled && (now - hazardExitLeftStart >= PRESS_TIME)) {
      hazardMode = false;
      hazardExitLeftHandled = true;
      Serial.println("[MODE CHANGE] >>> HAZARD OFF (LEFT) <<<");
    }
  } else {
    hazardExitLeftStart = 0;
    hazardExitLeftHandled = false;
  }
}

void updateLEDs() {
  if (hazardMode) {
    digitalWrite(led1, blinkState);
    digitalWrite(led2, blinkState);

    Serial.print("[LED] HAZARD → R=");
    Serial.print(blinkState ? "ON" : "OFF");
    Serial.print(" L=");
    Serial.println(blinkState ? "ON" : "OFF");
  } else {
    digitalWrite(led1, rightIndicator ? blinkState : LOW);
    digitalWrite(led2, leftIndicator ? blinkState : LOW);

    Serial.print("[LED] R=");
    Serial.print(rightIndicator ? "ON" : "OFF");
    Serial.print(" L=");
    Serial.print(leftIndicator ? "ON" : "OFF");
    Serial.print(" Blink=");
    Serial.println(blinkState ? "ON" : "OFF");
  }

  delay(500);
}
