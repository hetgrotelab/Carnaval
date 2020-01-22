void keypad()
{
// Draw the upper row of buttons
but1 = myButtons.addButton( 40, 95, 50, 50, "1");
but2 = myButtons.addButton( 95, 95, 50, 50, "2");
but3 = myButtons.addButton( 150, 95, 50, 50, "3");
but4 = myButtons.addButton( 40, 150, 50, 50, "4");
but5 = myButtons.addButton( 95, 150, 50, 50, "5");
but6 = myButtons.addButton( 150, 150, 50, 50, "6");
but7 = myButtons.addButton( 40, 205, 50, 50, "7");
but8 = myButtons.addButton( 95, 205, 50, 50, "8");
but9 = myButtons.addButton( 150, 205, 50, 50, "9");
but10 = myButtons.addButton( 95, 260, 50, 50, "0");
butClr = myButtons.addButton( 40, 260, 50, 50, "C");
myButtons.drawButtons();

butEnt = myButtons.addButton( 150, 260, 50, 50, "Ok");
myButtons.drawButton(butEnt);
}

void updateStr(int val)
{
 if (stCurrentLen<20)
 {
   stCurrent[stCurrentLen]=val;
   stCurrent[stCurrentLen+1]='\0';
   stCurrentLen++;
   myGLCD.setColor(0, 255, 0);
   myGLCD.print(stCurrent, LEFT, 224);
 }
 else
 {
   myGLCD.setColor(255, 0, 0);
   myGLCD.print("BUFFER FULL!", CENTER, 192);
   delay(500);
   myGLCD.print("            ", CENTER, 192);
   delay(500);
   myGLCD.print("BUFFER FULL!", CENTER, 192);
   delay(500);
   myGLCD.print("            ", CENTER, 192);
   myGLCD.setColor(0, 255, 0);
 }
}
// Draw a red frame while a button is touched
void waitForIt(int x1, int y1, int x2, int y2)
{
 myGLCD.setColor(255, 0, 0);
 myGLCD.drawRoundRect (x1, y1, x2, y2);
 while (myTouch.dataAvailable())
   myTouch.read();
 myGLCD.setColor(255, 255, 255);
 myGLCD.drawRoundRect (x1, y1, x2, y2);
}
