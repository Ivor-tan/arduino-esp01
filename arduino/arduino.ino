
int ledWhite = 6;
int ledRed = 5;
int ledGreen = 3;
int ledYellow = 9;
int ledBlue = 11;

bool WhiteState = false;
bool RedState = false;
bool GreenState = false;
bool YellowState = false;
bool BlueState = false;

int WhiteStrength = 200;
int RedStrength = 200;
int GreenStrength = 200;
int YellowStrength = 200;
int BlueStrength = 200;

String Esp_String = "";
int commendIndex = 1;
void setup()
{
    Serial.begin(9600);
    pinMode(ledWhite, OUTPUT);
    pinMode(ledRed, OUTPUT);
    pinMode(ledGreen, OUTPUT);
    pinMode(ledYellow, OUTPUT);
    pinMode(ledBlue, OUTPUT);

    Serial.println("Esp_String=============>");
}

void loop()
{
    if (Serial.available() > 0)
    {
        if (Serial.peek() != '&')
        {
            Esp_String += (char)Serial.read();
        }
        else
        {
            Serial.read();
            Serial.println(Esp_String);
            Esp_String.trim();
            if (Esp_String.startsWith("<") && Esp_String.endsWith(">"))
            {
                dealMsg();
            }
            Esp_String = "";
        }
    }
    workLight();
}

void dealMsg()
{
    String commend = Esp_String.substring(1, Esp_String.length() - 1); // 截取命令字符
    String commendColor = "";
    String commendState = "";
    String commendStrength = "";

    char *token;                                  // 存放每次切割得到的子字符串指针
    token = strtok((char *)commend.c_str(), "-"); // 分割三部分指令
    while (token != NULL)
    {

        if (commendIndex == 1)
        {
            commendColor += token;
            // 控制灯的颜色
            Serial.println(commendColor);
        }
        if (commendIndex == 2)
        {
            commendState += token;
            // 控制灯的开关
            Serial.println(commendState);
        }
        if (commendIndex == 3)
        {
            commendStrength += token;
            // 控制灯的亮度
            Serial.println(commendStrength);
        }
        lightChange(commendColor, commendState, commendStrength);
        commendIndex++;
        token = strtok(NULL, "-"); // 获取下一个子字符串
    }
    commendIndex = 1;
}

void workLight()
{
    if (WhiteState)
    {
        analogWrite(ledWhite, WhiteStrength);
        // digitalWrite(ledWhite, HIGH);
    }
    else
    {
        digitalWrite(ledWhite, HIGH);
    }

    if (RedState)
    {
        //  digitalWrite(ledRed, HIGH);
        analogWrite(ledRed, RedStrength);
    }
    else
    {
        digitalWrite(ledRed, HIGH);
    }


    if (GreenState)
    {
        analogWrite(ledGreen, GreenStrength);
    }
    else
    {
        digitalWrite(ledGreen, HIGH);
    }

    if (YellowState)
    {
        analogWrite(ledYellow, YellowStrength);
    }
    else
    {
        digitalWrite(ledYellow, HIGH);
    }

    if (BlueState)
    {
        analogWrite(ledBlue, BlueStrength);
    }
    else
    {
        digitalWrite(ledBlue, HIGH);
    }

}

void lightChange(String Color, String State, String Strength)
{
    if (Color.equals("White"))
    {
        WhiteState = State.equals("ON");
        if (WhiteState)
        {
            WhiteStrength = Strength.toInt();
        }
    }

    if (Color.equals("Yellow"))
    {
        YellowState = State.equals("ON");
        if (YellowState)
        {
            YellowStrength = Strength.toInt();
        }
    }

    if (Color.equals("Green"))
    {
        GreenState = State.equals("ON");
        if (GreenState)
        {
            GreenStrength = Strength.toInt();
        }
    }
    if (Color.equals("Blue"))
    {
        BlueState = State.equals("ON");
        if (BlueState)
        {
            BlueStrength = Strength.toInt();
        }
    }

     if (Color.equals("Red"))
    {
        RedState = State.equals("ON");
        if (RedState)
        {
            RedStrength = Strength.toInt();
        }
    }
}