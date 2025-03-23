#include <Arduino.h>
#include <UTFT.h>
#include <URTouch.h>
#include <UTFT_Buttons.h>
// #include <HardwareSerial.h>
// #include <Wire.h>

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];
extern uint8_t SevenSegNumFont[];

UTFT myGLCD(CTE32_R2, 38, 39, 40, 41);
// UTFT myGLCD(ILI9341_16, 38, 39, 40, 41);

enum active_page
{
  ERROR_PAGE,
  NO_WIFI_PAGE,
  HOMEPAGE,
  MODAL_PAGE,
  BUTTONS_PAGE,
  SUCCESS_VERIFICATION_PAGE,
  WAIT_PAYMENT_PAGE,
  RETRY_PAYMENT_PAGE,
  PAYMENT_INFO_PAGE
};

int active_page = NO_WIFI_PAGE;
int available_slots = 10;
URTouch myTouch(6, 5, 4, 3, 2);

UTFT_Buttons myButtons(&myGLCD, &myTouch);
UTFT_Buttons myModalButtons(&myGLCD, &myTouch);
UTFT_Buttons goToPaymentButtons(&myGLCD, &myTouch);
UTFT_Buttons resetButton(&myGLCD, &myTouch);
int key1, key2, key3, key4, key5, key6, key7, key8, key9, key0, clear, enter, pressed_buttons, yes_btn, cancel_btn;
String enterTime, exitTime;
String cardId;
int payment_amount;
int confirm_pay, reset_wifi_btn;
String number;
void handle_buttons_touch();
void handle_serial_data();
void draw_home();
void draw_success_verify();
void draw_enter_button_interface();
void drawButtons();
void show_number();
void show_confirm_number();
void draw_wait_payment_success_page();
void draw_prompt_payment_page();
void draw_show_payment_info();
void clear_screen();
void draw_no_wifi_screen();
void draw_error_page();
int parseTimeToSeconds(String time);
void displayElapsedTime(String enterTime, String exitTime);

void setup()
{
  Serial.begin(9600);
  Serial2.begin(115200);
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch();
  delay(500);
  myTouch.setPrecision(PREC_MEDIUM);

  myButtons.setTextFont(BigFont);
  // myButtons.setTextFont(BigFont);
  // myButtons.setSymbolFont(Dingbats1_XL);

  // draw_home();
  draw_no_wifi_screen();
  // draw_show_payment_info();
  // draw_prompt_payment_page();
  // draw_wait_payment_success_page();
  // draw_success_verify();
  // drawButtons();
  // show_confirm_number();
}

void loop()
{

  while (true)
  {
    handle_serial_data();
    // handle_buttons_touch();
  }
}

void handle_serial_data()
{

  if (Serial2.available())
  {
    Serial.println("Serial 2 is available");
    String command = Serial2.readStringUntil('\n');
    Serial.print(F("Received command now: "));
    Serial.println(command);
    if (command.startsWith("NO_WIFI") && active_page != NO_WIFI_PAGE)
    {
      active_page = NO_WIFI_PAGE;
      clear_screen();
      draw_no_wifi_screen();
    }
    else if (command.startsWith("IDLE") && active_page != HOMEPAGE)
    {
      Serial.println("What the fuck is happening here?");
      clear_screen();
      active_page = HOMEPAGE;
      Serial.println(command.substring(5));
      if ((command.substring(5)).startsWith("PARKED"))
      {
        Serial.println("Yessss");
        available_slots -= 1;
      }
      else if ((command.substring(5)).startsWith("EXITED"))
      {
        Serial.println("Yessssooooo");
        available_slots += 1;
      }
      draw_home();
    }
    else if (command == "ENTERING")
    {
    }
    else if (command == "VERIFIED_ENTRANCE")
    {
    }
    else if (command == "WAITING_VERIFICATION")
    {
    }
    else if (command.startsWith("CARD_INFO_AT_VERIFY:") && active_page != SUCCESS_VERIFICATION_PAGE)
    {
      active_page = SUCCESS_VERIFICATION_PAGE;
      Serial.print(F("Card info at verify: "));
      Serial.println(command.substring(21));
      clear_screen();
      draw_success_verify();
      if (command.substring(20).startsWith("f658c096"))
      {
        Serial.println("Phone registered successfully");
        enterTime = command.substring(20 + 8 + 10);
        Serial.print(F("Enter time: "));
        Serial.println(enterTime);
        myGLCD.print("Entered at: " + enterTime, 10, 170);
        myGLCD.print("Plate number: RAB0005", 10, 140);
        myGLCD.print("Registered to: Rukundo", 10, 155);
      }
      else if (0)
      {
      }
      else
      {
      }
      {
        myGLCD.print("", 20, 30);
      }
    }
    else if (command.startsWith("WAITING_PAYMENT") && active_page != WAIT_PAYMENT_PAGE)
    {

      Serial.println("Waiting payment was called");
      active_page = WAIT_PAYMENT_PAGE;
      clear_screen();
      // draw_prompt_payment_page();
      draw_error_page();
    }
    else if (command.startsWith("RETRY_PAYMENT") && active_page != RETRY_PAYMENT_PAGE)
    {
      active_page = RETRY_PAYMENT_PAGE;
      clear_screen();
      draw_prompt_payment_page();
    }
    else if (command.startsWith("CARD_INFO_AT_PAY:") && active_page != PAYMENT_INFO_PAGE)
    {
      // Extract cardId, exitTime, and enterTime from the command string
      cardId = command.substring(17, 25);
      Serial.print(F("Card id: "));
      Serial.println(cardId);

      int exitTimeIndex = command.indexOf("exitTime:") + 9;
      exitTime = command.substring(exitTimeIndex, exitTimeIndex + 8);
      Serial.print(F("Exit time: "));
      Serial.println(exitTime);

      int enterTimeIndex = command.indexOf("enterTime:") + 10;
      enterTime = command.substring(enterTimeIndex, enterTimeIndex + 8);
      Serial.print(F("Enter time: "));
      Serial.println(enterTime);

      active_page = PAYMENT_INFO_PAGE;
      myGLCD.clrScr();
      myGLCD.fillScr(0, 0, 0);
      delay(200);
      draw_show_payment_info();
      if (cardId.startsWith("f658c096"))
      {
        displayElapsedTime(enterTime, exitTime);
        delay(5000);
        String commandToESP = "PROCEED:0788837266";
        Serial2.println(commandToESP);
      }
      else if (cardId.startsWith("b13e737f"))
      {
        displayElapsedTime(enterTime, exitTime);
        delay(5000);
        String commandToESP = "PROCEED:0788837266-AMOUNT:" + String(payment_amount);
        Serial2.println(commandToESP);
      }
      else
      {
      }
    }
    else if (command == "EXITING")
    {
    }
    else if (command == "VERIFIED_PAYMENT")
    {
    }
    else if (command == "HAS_PARKED")
    {
    }
    else if (command == "HAS_EXITED")
    {
      // active_page = HOMEPAGE;
      // clear_screen();
      // draw_home();
    }
    else if (command.startsWith("ERROR"))
    {
      active_page = ERROR_PAGE;
      clear_screen();
      draw_error_page();
    }
  }
}
void handle_buttons_touch()
{

  if (myTouch.dataAvailable())
  {

    Serial.println("Touch data is available");
    int x = myTouch.getX();
    int y = myTouch.getY();
    Serial.print("X: ");
    Serial.println(x);
    Serial.print("Y: ");
    Serial.println(y);
    switch (active_page)
    {
    case BUTTONS_PAGE:
      pressed_buttons = myButtons.checkButtons();
      Serial.print("Prressed button is: ");
      Serial.println(pressed_buttons);
      if (pressed_buttons == key0)
      {
        number.concat("0");
        show_number();
      }
      else if (pressed_buttons == key1)
      {
        number.concat("1");
        show_number();
      }
      else if (pressed_buttons == key2)
      {
        number.concat("2");
        show_number();
      }
      else if (pressed_buttons == key3)
      {
        number.concat("3");
        show_number();
      }
      else if (pressed_buttons == key4)
      {
        number.concat("4");
        show_number();
      }
      else if (pressed_buttons == key5)
      {
        number.concat("5");
        show_number();
      }
      else if (pressed_buttons == key6)
      {
        number.concat("6");
        show_number();
      }
      else if (pressed_buttons == key7)
      {
        number.concat("7");
        show_number();
      }
      else if (pressed_buttons == key8)
      {
        number.concat("8");
        show_number();
      }
      else if (pressed_buttons == key9)
      {
        number.concat("9");
        show_number();
      }
      else if (pressed_buttons == clear)
      {
        number = number.substring(0, number.length() - 1);
        show_number();
      }
      else if (pressed_buttons == enter)
      {

        clear_screen();
        active_page = MODAL_PAGE;
        show_confirm_number();
      }
      break;
    case MODAL_PAGE:
      Serial.print("Prressed button is: ");
      Serial.println(pressed_buttons);
      pressed_buttons = myModalButtons.checkButtons();
      if (pressed_buttons == cancel_btn)
      {
        clear_screen();
        active_page = BUTTONS_PAGE;
        drawButtons();
        show_number();
        // myButtons.drawButtons();
      }
      else if (pressed_buttons == yes_btn)
      {
        clear_screen();
        active_page = WAIT_PAYMENT_PAGE;
        draw_wait_payment_success_page();
        String commandToESP = "PROCEED:" + number;
        Serial2.println(commandToESP);
      }
    case PAYMENT_INFO_PAGE:
      pressed_buttons = goToPaymentButtons.checkButtons();
      if (pressed_buttons == confirm_pay)
      {
        clear_screen();
        active_page = BUTTONS_PAGE;
        drawButtons();
      }
    case NO_WIFI_PAGE:
      pressed_buttons = resetButton.checkButtons();
      if (pressed_buttons == reset_wifi_btn)
      {
        Serial2.println("RESET");
        delay(200);
        // reset arduino via soft reset
        asm volatile("  jmp 0");
      }
      break;
    default:
      break;
    }
    delay(200);
  }
}
void draw_home()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(SmallFont);
  myGLCD.print("WELCOME TO THE ", CENTER, 10);
  myGLCD.setFont(BigFont);
  myGLCD.print("KIGALI CITY MARKET", CENTER, 30);
  myGLCD.print("PARKING", CENTER, 50);
  myGLCD.setColor(182, 247, 2);
  myGLCD.setFont(BigFont);
  myGLCD.print("Available Slots: ", CENTER, 100);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print(String(available_slots), CENTER, 120);
  myGLCD.setFont(SmallFont);
  myGLCD.print("To continue tap your card", CENTER, 220);
}

void draw_success_verify()
{
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setFont(BigFont);
  myGLCD.print("Thank you!", CENTER, 20);
  myGLCD.setFont(SmallFont);
  myGLCD.print("You can now proceed to parking.", CENTER, 60);
  myGLCD.setColor(182, 247, 2);
  myGLCD.print("Car info: ", 10, 115);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawLine(LEFT, 130, RIGHT, 131);
  // myGLCD.print("Plate number: ", 10, 140);
  // myGLCD.print("Registered to: ", 10, 155);
  // myGLCD.print("Phone number: ", 10, 170);
}

void drawButtons()
{
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
  myButtons.setTextFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  int i = 0;
  key1 = myButtons.addButton(10 + (i * 60), 60, 50, 50, "1");
  key6 = myButtons.addButton(10 + (i * 60), 120, 50, 50, "6");
  i += 1;
  key2 = myButtons.addButton(10 + (i * 60), 60, 50, 50, "2");
  key7 = myButtons.addButton(10 + (i * 60), 120, 50, 50, "7");
  i += 1;
  key3 = myButtons.addButton(10 + (i * 60), 60, 50, 50, "3");
  key8 = myButtons.addButton(10 + (i * 60), 120, 50, 50, "8");
  i += 1;
  key4 = myButtons.addButton(10 + (i * 60), 60, 50, 50, "4");
  key9 = myButtons.addButton(10 + (i * 60), 120, 50, 50, "9");
  i += 1;
  key5 = myButtons.addButton(10 + (i * 60), 60, 50, 50, "5");
  key0 = myButtons.addButton(10 + (i * 60), 120, 50, 50, "0");

  clear = myButtons.addButton(10, 180, 140, 50, "Clear");
  enter = myButtons.addButton(160, 180, 140, 50, "Enter");
  delay(400);
  myButtons.drawButtons();
  // Serial.print("Active page: ");
  // Serial.println(active_page);
}

void show_confirm_number()
{
  Serial.println("Show confirm number was called");
  // myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Are you sure?", CENTER, 30);
  myGLCD.setFont(SmallFont);
  myGLCD.print("This is the number: ", CENTER, 50);
  myGLCD.setFont(BigFont);
  myGLCD.setColor(182, 247, 2);
  myGLCD.print(number, CENTER, 110);
  myModalButtons.setTextFont(BigFont);
  cancel_btn = myModalButtons.addButton(10, 175, 140, 50, "NO");
  yes_btn = myModalButtons.addButton(160, 175, 140, 50, "YES");
  myModalButtons.drawButtons();
  Serial.println("Show confirm number was called lastly");
}

void show_number()
{
  Serial.println(number);
  myGLCD.setFont(BigFont);
  myGLCD.setColor(182, 247, 2);
  myGLCD.print("                                    ", 10, 15);
  myGLCD.print(number, 10, 15);
}
void draw_wait_payment_success_page()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Check your phone!", CENTER, 20);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Your phone should prompt you to pay!", CENTER, 60);
  myGLCD.setColor(182, 247, 2);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Please wait...", CENTER, 100);
  myGLCD.print("Dial *182*7*1# if you don't see message directly", LEFT + 10, 220);
}

void draw_prompt_payment_page()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Tap your card", CENTER, 20);
  myGLCD.setFont(SmallFont);
  myGLCD.print("We need to verify your identity", CENTER, 40);
  myGLCD.print("Tap you card on a reader", CENTER, 200);
}
void draw_show_payment_info()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Parking info:", CENTER, 20);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Entered at: ", LEFT + 10, 50);
  myGLCD.print("Exited at: ", LEFT + 10, 80);
  myGLCD.print("Total Elapsed time:", LEFT + 10, 110);
  myGLCD.setFont(BigFont);
  myGLCD.print("Total ", LEFT + 10, 150);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.print("100", LEFT + 10, 180);
  myGLCD.setFont(BigFont);
  myGLCD.print("RWF", 140, 200);
  // goToPaymentButtons.setTextFont(BigFont);
  // confirm_pay = goToPaymentButtons.addButton(200, 180, 110, 50, "PAY");
  // goToPaymentButtons.drawButtons();
}
void clear_screen()
{
  myModalButtons.deleteAllButtons();
  goToPaymentButtons.deleteAllButtons();
  myButtons.deleteAllButtons();
  myGLCD.clrScr();
  myGLCD.fillScr(0, 0, 0);
  delay(200);
}
void draw_no_wifi_screen()
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("NO INTERNET", CENTER, 40);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(SmallFont);

  myGLCD.print("Please connect to wifi", CENTER, 80);
  myGLCD.print("System needs to be on internet.", LEFT + 10, 120);
  myGLCD.print("This is mainly because of the payment", LEFT + 10, 140);
  myGLCD.print("can't be made without it", LEFT + 10, 160);
  resetButton.setTextFont(BigFont);
  reset_wifi_btn = resetButton.addButton(100, 180, 150, 50, "REFRESH");
  resetButton.drawButtons();
}

void draw_error_page()
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("ERROR", CENTER, 40);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(SmallFont);

  myGLCD.print("An error occured", CENTER, 80);
  myGLCD.print("Please try again", CENTER, 120);
  myGLCD.print("If the problem persists", CENTER, 160);
  myGLCD.print("Please contact the admin", CENTER, 200);
}
int parseTimeToSeconds(String time)
{
  int hours = time.substring(0, 2).toInt();
  int minutes = time.substring(3, 5).toInt();
  int seconds = time.substring(6, 8).toInt();
  return (hours * 3600) + (minutes * 60) + seconds;
}

void displayElapsedTime(String enterTime, String exitTime)
{
  int enterSeconds = parseTimeToSeconds(enterTime);
  int exitSeconds = parseTimeToSeconds(exitTime);
  int elapsedTime = exitSeconds - enterSeconds;

  // Optionally convert elapsedTime back to HH:MM:SS format
  int hours = elapsedTime / 3600;
  int minutes = (elapsedTime % 3600) / 60;
  int seconds = elapsedTime % 60;

  String elapsedTimeStr = String(hours) + ":" + String(minutes) + ":" + String(seconds);
  myGLCD.setFont(SmallFont);
  myGLCD.print(elapsedTimeStr, LEFT + 100, 110);
  myGLCD.print("Entered at: " + enterTime, LEFT + 10, 50);
  myGLCD.print("Exited at: " + exitTime, LEFT + 10, 80);
  myGLCD.print("Total Elapsed time:" + elapsedTimeStr, LEFT + 10, 110);
  myGLCD.setFont(SevenSegNumFont);
  payment_amount = elapsedTime * 1;
  myGLCD.print(String(payment_amount), LEFT + 5, 180);
}