#define aPin 7
#define bPin 9
#define cPin 2
#define dPin 3
#define ePin 4
#define fPin 6
#define gPin 5

#define PIN_BUTTON A0
#define PIN_BUZZER 10
#define PIN_CHAOS A5

#define BEEP_FREQUENCY 4000

#define ON 1
#define OFF 0

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

int buttonPressed = 0;

unsigned long lastAnimationTime = 0;
const unsigned long animationInterval = 100;

int animationStep = 0;

void showNumber(int x);
void one(void);
void two(void);
void three(void);
void four(void);
void five(void);
void six(void);
void rollTheDice(void);

void setup(void)
{
    randomSeed(analogRead(PIN_CHAOS));

    pinMode(aPin, OUTPUT);
    pinMode(bPin, OUTPUT);
    pinMode(cPin, OUTPUT);
    pinMode(dPin, OUTPUT);
    pinMode(ePin, OUTPUT);
    pinMode(fPin, OUTPUT);
    pinMode(gPin, OUTPUT);

    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);

    int i;

    for (i = 1; i <= 6; i++)
    {
        showNumber(i);
        tone(PIN_BUZZER, BEEP_FREQUENCY, 100);
        delay(1000);
    }

    tone(PIN_BUZZER, BEEP_FREQUENCY, 600);
    delay(500);
}

void loop(void)
{
    unsigned long currentTime = millis();

    if (digitalRead(PIN_BUTTON) == LOW)
    {
        if (!buttonPressed &&
            (currentTime - lastDebounceTime > debounceDelay))
        {
            buttonPressed = 1;
            lastDebounceTime = currentTime;

            rollTheDice();
        }
    }
    else
    {
        buttonPressed = 0;
    }
}

void rollTheDice(void)
{
    animationStep = 0;

    while (animationStep < 10)
    {
        if (millis() - lastAnimationTime >= animationInterval)
        {
            lastAnimationTime = millis();

            int number = random(1, 7);

            showNumber(number);

            tone(PIN_BUZZER, BEEP_FREQUENCY, 5);

            animationStep++;
        }
    }

    int finalNumber = random(1, 7);

    showNumber(finalNumber);

    tone(PIN_BUZZER, BEEP_FREQUENCY, 300);
}

void showNumber(int x)
{
    switch (x)
    {
        case 1:
            one();
            break;

        case 2:
            two();
            break;

        case 3:
            three();
            break;

        case 4:
            four();
            break;

        case 5:
            five();
            break;

        case 6:
            six();
            break;
    }
}

void one(void)
{
    digitalWrite(aPin, OFF);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, OFF);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, OFF);
    digitalWrite(gPin, OFF);
}

void two(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, OFF);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, ON);
    digitalWrite(fPin, OFF);
    digitalWrite(gPin, ON);
}

void three(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, OFF);
    digitalWrite(gPin, ON);
}

void four(void)
{
    digitalWrite(aPin, OFF);
    digitalWrite(bPin, ON);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, OFF);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, ON);
    digitalWrite(gPin, ON);
}

void five(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, OFF);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, OFF);
    digitalWrite(fPin, ON);
    digitalWrite(gPin, ON);
}

void six(void)
{
    digitalWrite(aPin, ON);
    digitalWrite(bPin, OFF);
    digitalWrite(cPin, ON);
    digitalWrite(dPin, ON);
    digitalWrite(ePin, ON);
    digitalWrite(fPin, ON);
    digitalWrite(gPin, ON);
}